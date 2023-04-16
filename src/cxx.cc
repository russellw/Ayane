#include "main.h"

namespace {
enum
{
	k_ne = parser_k,
	k_le,
	k_ge,
	k_addAssign,
	k_subAssign,
	k_mulAssign,
	k_divAssign,
	k_remAssign,
	k_inc,
	k_dec,
	k_eq,
	k_and,
	k_or,

	k_if,
	k_while,
	k_else,
	k_do,
	k_for,
	k_return,
	k_int,
	k_bool,
	k_void,
	k_vector,
	k_val,
	k_set,
	k_map,
	k_graph,
	k_clause,
	k_break,
	k_continue,

	end_k
};

// Reserved words.
uint16_t reserved[end_s];

// Infix operators.
struct Op {
	unsigned char prec : 7;
	unsigned char left : 1;
};

Op ops[end_k];

int prec = 99;
inline void op(int k, int left = 1) {
	Op o;
	o.prec = prec;
	o.left = left;
	ops[k] = o;
}

struct init {
	init() {
		// Initialize reserved words.
		reserved[s_if] = k_if;
		reserved[s_for] = k_for;
		reserved[s_do] = k_do;
		reserved[s_while] = k_while;
		reserved[s_else] = k_else;
		reserved[s_return] = k_return;
		reserved[s_int] = k_int;
		reserved[s_bool] = k_bool;
		reserved[s_void] = k_void;
		reserved[s_vector] = k_vector;
		reserved[s_val] = k_val;
		reserved[s_set] = k_set;
		reserved[s_map] = k_map;
		reserved[s_graph] = k_graph;
		reserved[s_clause] = k_clause;
		reserved[s_break] = k_break;
		reserved[s_continue] = k_continue;

		// Initialize infix operators.
		op('*');
		op('/');
		op('%');

		prec--;
		op('+');
		op('-');

		prec--;
		op('<');
		op(k_le);
		op('>');
		op(k_ge);

		prec--;
		op(k_eq);
		op(k_ne);

		prec--;
		op(k_and);

		prec--;
		op(k_or);

		prec--;
		op('?', 0);

		prec--;
		op('=', 0);
		op(k_addAssign, 0);
		op(k_subAssign, 0);
		op(k_mulAssign, 0);
		op(k_divAssign, 0);
		op(k_remAssign, 0);
	}
} _;

struct parser1: parser {
	// Tokenizer.
	void lex() {
	loop:
		auto s = srck = src;
		switch (*s) {
		case ' ':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			src = s + 1;
			goto loop;
		case '!':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_ne;
				return;
			}
			break;
		case '#':
			do ++s;
			while (*s != '\n');
			goto loop;
		case '%':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_remAssign;
				return;
			}
			break;
		case '&':
			switch (s[1]) {
			case '&':
				src = s + 2;
				tok = k_and;
				return;
			}
			break;
		case '*':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_mulAssign;
				return;
			}
			break;
		case '+':
			switch (s[1]) {
			case '+':
				src = s + 2;
				tok = k_inc;
				return;
			case '=':
				src = s + 2;
				tok = k_addAssign;
				return;
			}
			break;
		case '-':
			switch (s[1]) {
			case '-':
				src = s + 2;
				tok = k_dec;
				return;
			case '=':
				src = s + 2;
				tok = k_subAssign;
				return;
			}
			break;
		case '/':
			switch (s[1]) {
			case '*':
				for (s += 2; !(*s == '*' && s[1] == '/'); ++s)
					if (!*s) err("Unclosed comment");
				src = s + 2;
				goto loop;
			case '/':
				do ++s;
				while (*s != '\n');
				goto loop;
			case '=':
				src = s + 2;
				tok = k_divAssign;
				return;
			}
			break;
		case '<':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_le;
				return;
			}
			break;
		case '=':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_eq;
				return;
			}
			break;
		case '>':
			switch (s[1]) {
			case '=':
				src = s + 2;
				tok = k_ge;
				return;
			}
			break;
		case '_':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			word();
			return;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		{
			word();
			auto i = keyword(str);
			if (i < end_s && reserved[i]) tok = reserved[i];
			return;
		}
		case '|':
			switch (s[1]) {
			case '|':
				src = s + 2;
				tok = k_or;
				return;
			}
			break;
		case 0:
			tok = 0;
			return;
		}
		src = s + 1;
		tok = *s;
	}

	bool eat(int k) {
		if (tok == k) {
			lex();
			return 1;
		}
		return 0;
	}

	void expect(char k) {
		if (eat(k)) return;
		sprintf(buf, "Expected '%c'", k);
		err(buf);
	}

	void expect(int k, const char* s) {
		if (eat(k)) return;
		sprintf(buf, "Expected %s", s);
		err(buf);
	}

	// Types.
	bool isType() {
		switch (tok) {
		case k_bool:
		case k_clause:
		case k_graph:
		case k_int:
		case k_map:
		case k_set:
		case k_val:
		case k_vector:
		case k_void:
			return 1;
		}
		return 0;
	}

	type typ() {
		switch (tok) {
		case k_bool:
			lex();
			return kind::Bool;
		case k_clause:
		{
			lex();
			auto ty = type(kind::List, kind::Term);
			return type(kind::Tuple, ty, ty);
		}
		case k_graph:
			lex();
			return type(kind::Set, type(kind::Tuple, kind::Term, kind::Term));
		case k_int:
			lex();
			return kind::Integer;
		case k_map:
		{
			lex();
			expect('<');
			auto k = typ();
			expect(',');
			auto v = typ();
			expect('>');
			return type(kind::Map, k, v);
		}
		case k_set:
		{
			lex();
			expect('<');
			auto ty = typ();
			expect('>');
			return type(kind::Set, ty);
		}
		case k_val:
			lex();
			return kind::Term;
		case k_vector:
		{
			lex();
			expect('<');
			auto ty = typ();
			expect('>');
			return type(kind::List, ty);
		}
		case k_void:
			lex();
			return kind::Void;
		}
		err("Expected type");
	}

	// Expressions.
	term primary() {
		switch (tok) {
		case '(':
		{
			lex();
			auto a = expr();
			expect(')');
			return a;
		}
		}
		err("Expected expression");
	}

	term postfix() {
		auto a = primary();
		for (;;) switch (tok) {
			case k_inc:
				lex();
				return term(tag::PostInc, a, integer(1));
			case k_dec:
				lex();
				return term(tag::PostInc, a, integer(-1));
			case '[':
				lex();
				a = term(tag::Subscript, a, expr());
				expect(']');
				break;
			default:
				return a;
			}
	}

	term prefix() {
		switch (tok) {
		case '!':
			lex();
			return term(tag::Not, prefix());
		case '-':
			lex();
			return term(tag::Neg, prefix());
		case k_dec:
			lex();
			return term(tag::OpAssign, term(tag::Sub, postfix(), integer(1)));
		case k_inc:
			lex();
			return term(tag::OpAssign, term(tag::Add, postfix(), integer(1)));
		}
		return postfix();
	}

	term infix(int prec) {
		auto a = prefix();
		for (;;) {
			auto k = tok;
			auto o = ops[k];
			if (!o.prec) break;
			if (o.prec <= prec) break;
			lex();
			auto b = infix(o.prec + o.left);
			switch (k) {
			case '%':
				a = term(tag::RemT, a, b);
				break;

			case '*':
				a = term(tag::Mul, a, b);
				break;
			case '+':
				a = term(tag::Add, a, b);
				break;
			case '-':
				a = term(tag::Sub, a, b);
				break;

			case '/':
				a = term(tag::Div, a, b);
				break;
			case '<':
				a = term(tag::Lt, a, b);
				break;
			case '=':
				a = term(tag::Assign, a, b);
				break;
			case '>':
				a = term(tag::Lt, b, a);
				break;
			case '?':
				expect(':');
				a = term(tag::IfExpr, a, b, infix(o.prec + o.left));
				break;

			case k_addAssign:
				a = term(tag::OpAssign, term(tag::Add, a, b));
				break;
			case k_and:
				a = term(tag::And, a, b);
				break;

			case k_divAssign:
				a = term(tag::OpAssign, term(tag::Div, a, b));
				break;
			case k_eq:
				a = term(tag::Eq, a, b);
				break;
			case k_ge:
				a = term(tag::Le, b, a);
				break;

			case k_le:
				a = term(tag::Le, a, b);
				break;
			case k_mulAssign:
				a = term(tag::OpAssign, term(tag::Mul, a, b));
				break;
			case k_ne:
				a = term(tag::Not, term(tag::Eq, a, b));
				break;

			case k_or:
				a = term(tag::Or, a, b);
				break;

			case k_remAssign:
				a = term(tag::OpAssign, term(tag::RemT, a, b));
				break;

			case k_subAssign:
				a = term(tag::OpAssign, term(tag::Sub, a, b));
				break;
			default:
				unreachable;
			}
		}
		return a;
	}

	term expr() {
		return infix(0);
	}

	// Statements.
	term stmt() {
		switch (tok) {
		case k_break:
			lex();
			expect(';');
			return term(tag::Break);
		case k_continue:
			lex();
			expect(';');
			return term(tag::Continue);
		case k_do:
		{
			lex();
			auto body = stmt();
			expect(k_while, "expected 'while'");
			expect('(');
			auto cond = expr();
			expect(')');
			expect(';');
			return term(tag::DoWhile, body, cond);
		}
		case k_for:
		{
			lex();
			expect('(');
			term init;
			if (tok != ';') init = expr();
			expect(';');
			term cond;
			if (tok != ';') cond = expr();
			expect(';');
			term update;
			if (tok != ')') update = expr();
			expect(')');
			return term(tag::For, init, cond, update, stmt());
		}
		case k_if:
		{
			lex();
			expect('(');
			auto cond = expr();
			expect(')');
			auto yes = stmt();
			if (eat(k_else)) return term(tag::If, cond, yes, stmt());
			return term(tag::If, cond, yes);
		}
		case k_return:
		{
			lex();
			if (eat(';')) return term(tag::ReturnVoid);
			auto a = expr();
			expect(';');
			return term(tag::Return, a);
		}
		case k_while:
		{
			lex();
			expect('(');
			auto cond = expr();
			expect(')');
			return term(tag::While, cond, stmt());
		}
		}
		auto a = expr();
		expect(';');
		return a;
	}

	parser1(const char* file): parser(file) {
		lex();
	}
};
} // namespace
