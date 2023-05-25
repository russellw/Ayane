#include "main.h"

namespace {
enum {
	k_keyword = ntoks,
	k_string,
};

char issym[0x100];

struct Parser1: Parser {
	// Tokenizer
	void lex() {
	loop:
		auto s = srck = src;
		switch (*s) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			src = s + 1;
			goto loop;
		case '!':
		case '$':
		case '%':
		case '&':
		case '*':
		case '+':
		case '-':
		case '.':
		case '/':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case '^':
		case '_':
		case 'A':
		case 'a':
		case 'B':
		case 'b':
		case 'C':
		case 'c':
		case 'D':
		case 'd':
		case 'E':
		case 'e':
		case 'F':
		case 'f':
		case 'G':
		case 'g':
		case 'H':
		case 'h':
		case 'I':
		case 'i':
		case 'J':
		case 'j':
		case 'K':
		case 'k':
		case 'L':
		case 'l':
		case 'M':
		case 'm':
		case 'N':
		case 'n':
		case 'O':
		case 'o':
		case 'P':
		case 'p':
		case 'Q':
		case 'q':
		case 'R':
		case 'r':
		case 'S':
		case 's':
		case 'T':
		case 't':
		case 'U':
		case 'u':
		case 'V':
		case 'v':
		case 'W':
		case 'w':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case 'Z':
		case 'z':
		case '~':
		{
			// TODO: the order of cases is weird
			auto s = src;
			do ++s;
			while (issym[*(unsigned char*)s]);
			str = intern(src, s - src);
			src = s;
			tok = k_word;
			return;
		}
		case '"':
		{
			++src;
			auto r = src;
			auto s = src;
			for (;;) {
				if (*s == '"') {
					if (s[1] != '"') break;
					++s;
				}
				if (!*s) err("unclosed '\"'");
				*r++ = *s++;
			}
			str = intern(src, r - src);
			src = s + 1;
			tok = k_string;
			return;
		}
		case '#':
			err("bit vectors not supported", inappropriateError);
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			// TODO: Although the various kinds of literals have some conventional, intuitive meanings, their semantics is actually determined by the definition of the logic being used. <decimal>s might represent rationals in one logic and reals in another; <binary>s might be bit sequences in one logic and integers in another; even <string>s might be interpretable as integers or sequences.
			number();
			return;
		case ':':
			do ++s;
			while (issym[*(unsigned char*)s]);

			// We don't use the actual keyword
			src = s;
			tok = k_word;
			return;
		case ';':
			src = strchr(s, '\n');
			goto loop;
		case '|':
		{
			++src;
			auto r = src;
			auto s = src;
			while (*s != '|') {
				if (!*s) err("unclosed '|'");
				*r++ = *s++;
			}
			str = intern(src, r - src);
			src = s + 1;
			tok = k_word;
			return;
		}
		}
		src = s + 1;
		tok = *s;
	}

	// Parser
	bool eat(int k) {
		if (tok == k) {
			lex();
			return 1;
		}
		return 0;
	}

	void expect(char k) {
		if (eat(k)) return;
		sprintf(buf, "expected '%c'", k);
		err(buf);
	}

	Str* word() {
		if (tok != k_word) err("expected word");
		auto s = str;
		lex();
		return s;
	}

	// Types
	LeafType* atomicType() {
		switch (word() - keywords) {
		case s_Int:
			return &tinteger;
		default:
			err("unknown type");
		}
	}

	Type* topLevelType() {
		expect('(');

		if (eat(')')) return atomicType();

		Vec<Type*> v(1);
		do v.add(atomicType());
		while (!eat(')'));
		v[0] = atomicType();
		return compType(v);
	}

	// Expressions
	Expr* expr(Tag tag) {
		if (tok == ')') err("expected args");

		Vec<Expr*> v;
		do v.add(expr());
		while (!eat(')'));
		return comp(tag, v);
	}

	Expr* expr() {
		auto k = tok;
		auto s = str;
		auto num1 = num;
		lex();
		switch (k) {
		case '(':
		{
			s = word();
			switch (s - keywords) {
			case s_and:
				return expr(Tag::and1);
			case s_ge:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::or1, comp(Tag::eq, b, a), comp(Tag::lt, b, a));
			}
			case s_gt:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::lt, b, a);
			}
			case s_imp:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return imp(a, b);
			}
			case s_le:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::or1, comp(Tag::eq, a, b), comp(Tag::lt, a, b));
			}
			case s_lt:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::lt, a, b);
			}
			case s_minus:
				return expr(Tag::sub);
			case s_not:
				return expr(Tag::not1);
			case s_or:
				return expr(Tag::or1);
			case s_plus:
				return expr(Tag::add);
			case s_slash:
				// TODO: check types for integer division
				return expr(Tag::div);
			case s_star:
				return expr(Tag::mul);
			case s_xor:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::not1, comp(Tag::eqv, a, b));
			}
			}
		}
		case k_num:
			// TODO: this is not quite correct; in the reals theory, integer literals actually have real type
			return num1;
		case k_word:
			switch (s - keywords) {
			case s_false:
				return bools;
			case s_true:
				return bools + 1;
			}
			return fn(s, 0);
		}
		err("expected expression");
	}

	// Top level
	void skip() {
		while (!eat(')')) {
			if (!tok) err("unclosed '('");
			lex();
		}
	}

	Parser1(const char* file): Parser(file) {
		lex();
		while (tok) {
			expect('(');
			switch (word() - keywords) {
			case s_assert:
			{
				auto a = expr();
				expect(')');
				typing(a, &tbool);
				cnf(a);
				break;
			}
			case s_check_sat:
				return;
			case s_declare_fun:
			{
				auto s = word();
				fn(s, topLevelType());
				expect(')');
				break;
			}
			case s_set_info:
			case s_set_logic:
				skip();
				break;
			default:
				err("unknown command");
			}
		}
	}
};
} // namespace

void smtlib(const char* file) {
	issym['!'] = 1;
	issym['$'] = 1;
	issym['%'] = 1;
	issym['&'] = 1;
	issym['*'] = 1;
	issym['+'] = 1;
	issym['-'] = 1;
	issym['.'] = 1;
	issym['/'] = 1;
	issym['<'] = 1;
	issym['='] = 1;
	issym['>'] = 1;
	issym['?'] = 1;
	issym['@'] = 1;
	issym['^'] = 1;
	issym['_'] = 1;
	issym['~'] = 1;
	memset(&issym['0'], 1, 10);
	memset(&issym['A'], 1, 26);
	memset(&issym['a'], 1, 26);
	Parser1 parser(file);
}
