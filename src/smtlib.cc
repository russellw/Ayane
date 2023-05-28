#include "main.h"

namespace {
enum {
	k_keyword = ntoks,
	k_string,
};

char issym[0x100];

Expr* eq(Expr* a, Expr* b) {
	auto tag = type(a) == &tbool ? Tag::eqv : Tag::eq;
	return comp(tag, a, b);
}

struct Parser1: Parser {
	Vec<pair<Str*, Expr*>> locals;

	// Tokenizer
	void lex() {
	loop:
		auto s = srck = src;
		switch (*s) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			++src;
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
		case '^':
		case '_':
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
		case '~':
			do ++s;
			while (issym[(unsigned char)*s]);
			str = intern(src, s - src);
			src = s;
			tok = k_word;
			return;
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
			while (issym[(unsigned char)*s]);

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
		++src;
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
	Type* type1() {
		switch (tok) {
		case '(':
			err("composite sorts not supported", inappropriateError);
		case k_word:
			auto s = str;
			lex();
			if (s->ty) return s->ty;
			switch (s - keywords) {
			case s_Bool:
				return &tbool;
			case s_Int:
				return &tinteger;
			case s_Real:
				return &treal;
			}
			err("unknown sort");
		}
		err("expected sort");
	}

	Type* topLevelType() {
		expect('(');

		if (eat(')')) return type1();

		Vec<Type*> v(1);
		do v.add(type1());
		while (!eat(')'));
		v[0] = type1();
		return compType(v);
	}

	// Expressions
	void params(Vec<Expr*>& v) {
		expect('(');
		while (!eat(')')) {
			expect('(');
			auto s = word();
			auto ty = type1();
			// TODO: do we need to check for Boolean parameters here?
			if (ty->kind == Kind::fn) err("higher-order functions not supported", inappropriateError);
			expect(')');
			auto a = var((LeafType*)ty, locals.n);
			locals.add(make_pair(s, a));
			v.add(a);
		}
	}

	Expr* quant(Tag tag) {
		Vec<Expr*> v(1);
		auto o = locals.n;
		params(v);
		v[0] = expr();
		expect(')');
		locals.n = o;
		return comp(tag, v);
	}

	Expr* expr(Tag tag) {
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
			if (s->fn) {
				Vec<Expr*> v(1, s->fn);
				do v.add(expr());
				while (!eat(')'));
				return comp(Tag::call, v);
			}
			switch (s - keywords) {
			case s_and:
				return expr(Tag::and1);
			case s_bvand:
			case s_bvnot:
			case s_bvor:
			case s_bvxor:
			case s_ite:
				sprintf(buf, "%s: not supported", s->v);
				err(buf, inappropriateError);
			case s_distinct:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::not1, eq(a, b));
			}
			case s_div:
				return expr(Tag::divEuclid);
			case s_eq:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return eq(a, b);
			}
			case s_exists:
				return quant(Tag::exists);
			case s_forall:
				return quant(Tag::all);
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
			case s_let:
			{
				expect('(');
				auto o = locals.n;
				do {
					expect('(');
					auto s = word();
					locals.add(make_pair(s, expr()));
					expect(')');
				} while (!eat(')'));
				auto a = expr();
				expect(')');
				locals.n = o;
				return a;
			}
			case s_lt:
			{
				auto a = expr();
				auto b = expr();
				expect(')');
				return comp(Tag::lt, a, b);
			}
			case s_minus:
			{
				auto a = expr();
				if (eat(')')) return comp(Tag::minus, a);
				auto b = expr();
				expect(')');
				return comp(Tag::sub, a, b);
			}
			case s_mod:
				return expr(Tag::remEuclid);
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
			snprintf(buf, bufSize, "'%s': not found", s->v);
			err(buf);
		}
		case k_num:
			// TODO: this is not quite correct; in the reals theory, integer literals actually have real type
			return num1;
		case k_word:
			// SMTLIB has lexical scope
			for (auto i = locals.rbegin(), e = locals.rend(); i != e; ++i)
				if (i->first == s) return i->second;

			// Function declarations have global scope
			if (s->fn) return s->fn;

			// There is the question of whether it should be possible to shadow true and false. They are not listed as reserved
			// words. SMT-LIB reference 2.6 says 'binders cannot shadow theory function symbols'. (This parser allows them to do so,
			// because that is simpler, but that is presumably a language extension.) But are true and false 'function' symbols?
			// Only if you count constants as functions of arity zero, which one might do in some contexts and not others. It does
			// seem that if we allow defined constants of those names at all, then they should shadow the built-in ones. Z3 4.12.1
			// allows such shadowing without complaint, so this parser follows suit.
			switch (s - keywords) {
			case s_false:
				return bools;
			case s_true:
				return bools + 1;
			}

			// Unlike TPTP, it's an error to use a name that has not been declared
			snprintf(buf, bufSize, "'%s': not found", s->v);
			err(buf);
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
			auto s = word();
			switch (s - keywords) {
			case s_assert:
			{
				bufp = buf;
				auto a = expr();
				typing(&tbool, a);
				expect(')');
				cnf(a);
				break;
			}
			case s_check_sat:
				return;
			case s_declare_datatypes:
				sprintf(buf, "%s: not supported", s->v);
				err(buf, inappropriateError);
			case s_declare_fun:
			{
				auto s = word();
				if (s->fn) err("function already declared");
				s->fn = new (ialloc(sizeof(Fn))) Fn(topLevelType(), s->v);
				expect(')');
				break;
			}
			case s_declare_sort:
			{
				auto s = word();
				if (s->ty) err("sort already declared");
				if (tok != k_num || num->tag != Tag::integer) err("expected arity");
				if (mpz_sgn(((Int*)num)->v)) err("sort parameters not supported", inappropriateError);
				lex();
				expect(')');
				s->ty = new (ialloc(sizeof(OpaqueType))) OpaqueType(s->v);
				break;
			}
			case s_define_fun:
			{
				// Name
				auto s = word();
				if (s->fn) err("function already declared");

				// Parameters
				// SORT
				auto o = locals.n;
				Vec<Expr*> v(1);
				///
				params(v);

				Vec<Type*> u(v.n);
				for (size_t i = 1; i < v.n; ++i) u[i] = ((Var*)v[i])->ty;

				// Return type
				u[0] = type1();

				// Declare
				s->fn = new (ialloc(sizeof(Fn))) Fn(compType(u), s->v);

				// Body
				bufp = buf;
				auto body = expr();
				locals.n = o;

				// Notional call
				Expr* call = s->fn;
				if (v.n > 1) {
					v[0] = call;
					call = comp(Tag::call, v);
				}

				// Define
				v[0] = eq(call, body);
				auto a = comp(Tag::all, v);
				typing(&tbool, a);
				expect(')');
				cnf(a);
				break;
			}
			case s_define_sort:
			{
				auto s = word();
				if (s->ty) err("sort already defined");
				s->ty = topLevelType();
				expect(')');
				break;
			}
			case s_push:
			case s_set_info:
			case s_set_logic:
				skip();
				break;
			default:
				snprintf(buf, bufSize, "'%s': unknown command", s->v);
				err(buf);
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
