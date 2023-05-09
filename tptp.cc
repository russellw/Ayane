#include "main.h"

namespace {
enum {
	k_distinctObj = ntoks,
	k_dollarWord,
	k_eqv,
	k_imp,
	k_impr,
	k_nand,
	k_ne,
	k_nor,
	k_var,
	k_xor,
};

unordered_map<Str*, Ex*> distinctObjs;

Ex* distinctObj(Str* s) {
	auto& a = distinctObjs[s];
	if (a) return a;
	a = (Ex*)malloc(offsetof(Ex, s) + sizeof(char*));
	a->tag = Individual;
	a->s = s->v;
	return a;
}

// If a term does not already have a type, assign it a specified one
void defaultType(Ex* a, Type* ty) {
	// A statement about the return type of a function call, can directly imply the type of the function. This generally does not
	// apply to basic operators; in most cases, they already have a definite type. That is not entirely true of the arithmetic
	// operators, but we don't try to do type inference to figure those out.
	if (a->tag == Call) {
		// TODO: might be some blank types in the arguments; is that okay, or do we need to error?
		ty = ftype(ty, a->v + 1, a->v + a->n);
		a = at(a, 0);
	}

	// This is only a default assignment, only relevant if the function does not already have a type
	if (a->tag == Fn && !a->ty) a->ty = ty;
}

// TODO: Which types should use const?
struct selection: unordered_set<const char*> {
	bool all;

	explicit selection(bool all): all(all) {
	}

	size_t count(const char* s) const {
		if (all) return 1;
		return unordered_set<const char*>::count(s);
	}
};

struct Parser1: Parser {
	// SORT
	bool cnfMode;
	const selection& sel;
	Vec<pair<Str*, Ex*>> vars;
	///

	// Tokenizer
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
		case '"':
			tok = k_distinctObj;
			quote();
			return;
		case '$':
			src = s + 1;
			word();
			tok = k_dollarWord;
			return;
		case '%':
			src = strchr(s, '\n');
			goto loop;
		case '+':
		case '-':
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
			num();
			return;
		case '/':
			switch (s[1]) {
			case '*':
				for (s += 2; !(*s == '*' && s[1] == '/'); ++s)
					if (!*s) err("Unclosed comment");
				src = s + 2;
				goto loop;
			}
			break;
		case '<':
			switch (s[1]) {
			case '=':
				if (s[2] == '>') {
					src = s + 3;
					tok = k_eqv;
					return;
				}
				src = s + 2;
				tok = k_impr;
				return;
			case '~':
				if (s[2] == '>') {
					src = s + 3;
					tok = k_xor;
					return;
				}
				err("Expected '>'");
			}
			break;
		case '=':
			switch (s[1]) {
			case '>':
				src = s + 2;
				tok = k_imp;
				return;
			}
			break;
		case '\'':
			tok = k_id;
			quote();
			return;
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
			tok = k_var;
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
			word();
			return;
		case '~':
			switch (s[1]) {
			case '&':
				src = s + 2;
				tok = k_nand;
				return;
			case '|':
				src = s + 2;
				tok = k_nor;
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

	// Types
	Type* atomicType() {
		auto k = tok;
		auto s = str;
		lex();
		switch (k) {
		case '(':
		{
			auto ty = atomicType();
			expect(')');
			return ty;
		}
		case k_dollarWord:
			switch (keyword(s)) {
			case s_i:
				return &tindividual;
			case s_int:
				return &tinteger;
			case s_o:
				return &tbool;
			case s_rat:
				return &trational;
			case s_real:
				return &treal;
			}
			break;
		case k_id:
			return type(s);
		}
		err("Expected type");
	}

	Type* topLevelType() {
		if (eat('(')) {
			// TODO: does vec(n) mean n or cap?
			Vec<Type*> v(1);
			do v.add(atomicType());
			while (eat('*'));
			expect(')');
			expect('>');
			v[0] = atomicType();
			return type(v);
		}
		auto ty = atomicType();
		if (eat('>')) return type(atomicType(), ty);
		return ty;
	}

	// Expressions
	void args(Vec<Ex*>& v) {
		expect('(');
		do v.add(atomicTerm());
		while (eat(','));
		expect(')');
	}

	Ex* definedFunctor(int tag) {
		Vec<Ex*> v;
		args(v);
		return ex(tag, v);
	}

	Ex* atomicTerm() {
		auto k = tok;
		auto s = str;
		auto sk = srck;
		auto end = src;
		lex();
		switch (k) {
		case k_distinctObj:
			return distinctObj(s);
		case k_dollarWord:
		{
			Vec<Ex*> v;
			switch (keyword(s)) {
			case s_ceiling:
				return definedFunctor(Ceil);
			case s_difference:
				return definedFunctor(Sub);
			case s_distinct:
			{
				args(v);
				for (auto& a: v) defaultType(a, &tindividual);
				Vec<Ex*> inequalities;
				for (auto i = v.begin(), e = v.end(); i < e; ++i)
					for (auto j = v.begin(); j != i; ++j) inequalities.add(ex(Not, ex(Eq, *i, *j)));
				return ex(And, inequalities);
			}
			case s_false:
				return bools;
			case s_floor:
				return definedFunctor(Floor);
			case s_greater:
				args(v);
				return ex(Lt, v[1], v[0]);
			case s_greatereq:
				args(v);
				return ex(Le, v[1], v[0]);
			case s_is_int:
				return definedFunctor(IsInteger);
			case s_is_rat:
				return definedFunctor(IsRational);
			case s_less:
				return definedFunctor(Lt);
			case s_lesseq:
				return definedFunctor(Le);
			case s_product:
				return definedFunctor(Mul);
			case s_quotient:
				return definedFunctor(Div);
			case s_quotient_e:
				return definedFunctor(DivE);
			case s_quotient_f:
				return definedFunctor(DivF);
			case s_quotient_t:
				return definedFunctor(DivT);
			case s_remainder_e:
				return definedFunctor(RemE);
			case s_remainder_f:
				return definedFunctor(RemF);
			case s_remainder_t:
				return definedFunctor(RemT);
			case s_round:
				return definedFunctor(Round);
			case s_sum:
				return definedFunctor(Add);
			case s_to_int:
				return definedFunctor(ToInteger);
			case s_to_rat:
				return definedFunctor(ToRational);
			case s_to_real:
				return definedFunctor(ToReal);
			case s_true:
				return bools + 1;
			case s_truncate:
				return definedFunctor(Trunc);
			case s_uminus:
				return definedFunctor(Neg);
			}
			break;
		}
		case k_id:
		{
			ex a(s, kind::Unknown);

			// Not a function call
			if (tok != '(') return a;

			// Function is being called, so gather the function and arguments
			Vec<Ex*> v(1, a);
			args(v);

			// By the TPTP specification, symbols can be assumed Boolean or individual, if not previously specified otherwise.
			// First-order logic does not allow functions to take Boolean arguments, so the arguments can default to individual. But
			// we cannot yet make any assumption about the function return type. For all we know here, it could still be Boolean.
			// Leave it to the caller, which will know from context whether that is the case.
			for (size_t i = 0; i < v.size(); ++i) defaultType(v[i], &tindividual);

			return ex(v);
		}
		case k_integer:
		{
			// It is more efficient to parse directly from the source buffer than to copy into a separate buffer first. The GMP
			// number parsers require a null terminator, so we supply one, overwriting the character immediately after the number
			// token. But it's possible that character was a newline, and later there will be an error that requires counting
			// newlines to report the line number, so we need to restore the character before returning.
			auto c = *end;
			*end = 0;
			auto a = integer(sk);
			*end = c;
			return a;
		}
		case k_rational:
		{
			auto c = *end;
			*end = 0;
			auto a = rational(sk);
			*end = c;
			return a;
		}
		case k_real:
		{
			auto c = *end;
			*end = 0;
			auto a = real(sk);
			*end = c;
			return a;
		}
		case k_var:
		{
			for (auto i = vars.rbegin(), e = vars.rend(); i < e; ++i)
				if (i->first == s) return i->second;
			if (!cnfMode) err("Unknown variable");
			auto x = var(vars.size(), &tindividual);
			vars.add(make_pair(s, x));
			return x;
		}
		}
		err("Expected expression");
	}

	Ex* infixUnary() {
		auto a = atomicTerm();
		switch (tok) {
		case '=':
		{
			lex();
			auto b = atomicTerm();
			defaultType(a, &tindividual);
			defaultType(b, &tindividual);
			return ex(Eq, a, b);
		}
		case k_ne:
		{
			lex();
			auto b = atomicTerm();
			defaultType(a, &tindividual);
			defaultType(b, &tindividual);
			return ex(Not, ex(Eq, a, b));
		}
		}
		defaultType(a, &tbool);
		return a;
	}

	Ex* quant(int tag) {
		lex();
		expect('[');
		auto old = vars.size();
		// TODO: check generated code
		Vec<Ex*> v{t, False};
		do {
			if (tok != k_var) err("Expected variable");
			auto s = str;
			lex();
			type ty = &tindividual;
			if (eat(':')) ty = atomicType();
			auto x = var(vars.size(), ty);
			vars.add(make_pair(s, x));
			v.add(x);
		} while (eat(','));
		expect(']');
		expect(':');
		v[1] = unary();
		vars.resize(old);
		return ex(v);
	}

	Ex* unary() {
		switch (tok) {
		case '!':
			return quant(All);
		case '(':
		{
			lex();
			auto a = logicFormula();
			expect(')');
			return a;
		}
		case '?':
			return quant(Exists);
		case '~':
			lex();
			return ex(Not, unary());
		}
		return infixUnary();
	}

	Ex* associativeLogicFormula(int tag, Ex* a) {
		Vec<Ex*> v{t, a};
		auto k = tok;
		while (eat(k)) v.add(unary());
		return ex(v);
	}

	Ex* logicFormula() {
		auto a = unary();
		switch (tok) {
		case '&':
			return associativeLogicFormula(And, a);
		case '|':
			return associativeLogicFormula(Or, a);
		case k_eqv:
			lex();
			return ex(Eqv, a, unary());
		case k_imp:
			lex();
			return imp(a, unary());
		case k_impr:
			lex();
			return imp(unary(), a);
		case k_nand:
			lex();
			return ex(Not, ex(And, a, unary()));
		case k_nor:
			lex();
			return ex(Not, ex(Or, a, unary()));
		case k_xor:
			lex();
			return ex(Not, ex(Eqv, a, unary()));
		}
		return a;
	}

	// Top level
	Str* wordOrDigits() {
		switch (tok) {
		case k_id:
		{
			auto r = str;
			lex();
			return r;
		}
		case k_num:
		{
			auto r = intern(srck, src - srck);
			lex();
			return r;
		}
		}
		err("Expected name");
	}

	void ignore() {
		switch (tok) {
		case '(':
			lex();
			while (!eat(')')) ignore();
			return;
		case 0:
			err("Too many '('s");
		}
		lex();
	}

	Parser1(const char* file, const selection& sel): Parser(file), sel(sel) {
		lex();
		while (tok) {
			// TODO: assert(!vars.n)
			vars.clear();
			auto kw = keyword(wordOrDigits());
			expect('(');
			auto name = wordOrDigits()->v;
			switch (kw) {
			case s_cnf:
			{
				expect(',');

				// Role
				wordOrDigits();
				expect(',');

				// Literals
				cnfMode = 1;
				auto a = quantify(logicFormula());

				// Select
				if (!sel.count(name)) break;

				// Clause
				problem.axiom(a, file, name);
				break;
			}
			case s_fof:
			case s_tff:
			{
				expect(',');

				// Role
				auto role = keyword(wordOrDigits());
				expect(',');

				// Type
				if (role == s_type) {
					size_t parens = 0;
					while (eat('(')) ++parens;

					auto s = wordOrDigits();
					expect(':');
					if (tok == k_dollarWord && str == keywords + s_tType) {
						// The symbol will be used as the name of a type. No particular action is required at this point, so accept
						// this and move on.
						lex();
					} else {
						// The symbol is the name of a function with the specified type. Call the term constructor that allows a
						// type to be specified, which will check for consistency.
						ex _(s, topLevelType());
					}

					while (parens--) expect(')');
					break;
				}

				// Formula
				cnfMode = 0;
				auto a = logicFormula();
				assert(vars.empty());
				check(a, &tbool);

				// Select
				if (!sel.count(name)) break;

				// Conjecture
				if (role == s_conjecture) {
					// If multiple conjectures occur in a problem, there are two possible interpretations (conjunction or
					// disjunction), and no consensus on which is correct, so rather than risk silently giving a wrong answer,
					// reject the problem as ambiguous and require it to be restated with the conjectures folded into one, using
					// explicit conjunction or disjunction
					if (conjecture) err("Multiple conjectures not supported");
					problem.conjecture(a, file, name);
					break;
				}

				// Ordinary formula
				problem.axiom(a, file, name);
				break;
			}
			case s_include:
			{
				auto dir = getenv("TPTP");
				if (!dir) err("TPTP environment variable not set");

				// File
				snprintf(buf, sizeof buf, "%s/%s", dir, name);
				auto file1 = intern(buf)->v;

				// Select and read
				if (eat(',')) {
					expect('[');

					selection sel1(0);
					do {
						auto selName = wordOrDigits();
						if (sel.count(selName->v)) sel1.add(selName->v);
					} while (eat(','));

					expect(']');
					Parser1 parser(file1, sel1, problem);
				} else {
					Parser1 parser(file1, sel, problem);
				}
				break;
			}
			default:
				err("Unknown language");
			}
			if (tok == ',') do
					ignore();
				while (tok != ')');
			expect(')');
			expect('.');
		}
	}
};
} // namespace

void tptp(const char* file, Problem& problem) {
	Parser1 parser(file, selection(1), problem);
}
