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

unordered_map<Str*, Expr*> distinctObjs;

Expr* distinctObj(Str* s) {
	auto& a = distinctObjs[s];
	if (a) return a;
	a = new (ialloc(sizeof(Expr))) Expr(Tag::distinctObj);
	return a;
}

struct Select: unordered_set<char*> {
	bool all;

	explicit Select(bool all): all(all) {
	}

	size_t count(char* s) const {
		if (all) return 1;
		return unordered_set<char*>::count(s);
	}
};

struct Parser1: Parser {
	// SORT
	bool cnfMode;
	const Select& select;
	Vec<pair<Str*, Expr*>> vars;
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
			lexWord();
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
			number();
			return;
		case '/':
			switch (s[1]) {
			case '*':
				for (s += 2; !(*s == '*' && s[1] == '/'); ++s)
					if (!*s) err("unclosed comment");
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
				err("expected '>'");
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
			tok = k_word;
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
			lexWord();
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
			lexWord();
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

	bool eatDollarWord(size_t i) {
		if (tok == k_dollarWord && str == keywords + i) {
			lex();
			return 1;
		}
		return 0;
	}

	// Types
	LeafType* atomicType() {
		auto k = tok;
		auto s = str;
		lex();
		switch (k) {
		case '!':
			err("type constructors not supported", inappropriateError);
		case '(':
		{
			auto ty = atomicType();
			expect(')');
			return ty;
		}
		case k_dollarWord:
			switch (s - keywords) {
			case s_i:
				return &tindividual;
			case s_int:
				return &tinteger;
			case s_o:
				return &tbool;
			case s_rat:
				return &trat;
			case s_real:
				return &treal;
			case s_tType:
				err("type constructors not supported", inappropriateError);
			}
			break;
		case k_word:
			return opaqueType(s);
		}
		err("expected type");
	}

	Type* topLevelType() {
		if (eat('(')) {
			Vec<Type*> v(1);
			do {
				auto ty = atomicType();
				if (ty == &tbool) err("$o is not a valid parameter type", inappropriateError);
				v.add(ty);
			} while (eat('*'));
			expect(')');
			expect('>');
			v[0] = atomicType();
			return compType(v);
		}
		auto ty = atomicType();
		if (!eat('>')) return ty;
		if (ty == &tbool) err("$o is not a valid parameter type", inappropriateError);
		return compType(atomicType(), ty);
	}

	// Expressions
	void args(Vec<Expr*>& v) {
		expect('(');
		do v.add(atomicTerm());
		while (eat(','));
		expect(')');
	}

	Expr* definedFunctor(Tag tag) {
		Vec<Expr*> v;
		args(v);
		return comp(tag, v);
	}

	Expr* atomicTerm() {
		auto k = tok;
		auto s = str;
		auto num1 = num;
		lex();
		switch (k) {
		case '(':
			err("formula not supported as expression", inappropriateError);
		case k_distinctObj:
			return distinctObj(s);
		case k_dollarWord:
		{
			switch (s - keywords) {
			case s_ceiling:
				return definedFunctor(Tag::ceil);
			case s_difference:
				return definedFunctor(Tag::sub);
			case s_distinct:
			{
				if (cnfMode) err("cnf does not support $distinct", typeError);
				Vec<Expr*> v;
				args(v);
				Vec<Expr*> inequalities;
				for (auto i = v.begin(), e = v.end(); i < e; ++i)
					for (auto j = v.begin(); j < i; ++j) inequalities.add(comp(Tag::not1, comp(Tag::eq, *i, *j)));
				return comp(Tag::and1, inequalities);
			}
			case s_false:
				return bools;
			case s_floor:
				return definedFunctor(Tag::floor);
			case s_greater:
			{
				expect('(');
				auto a = atomicTerm();
				expect(',');
				auto b = atomicTerm();
				expect(')');
				return comp(Tag::lt, b, a);
			}
			case s_greatereq:
			{
				expect('(');
				auto a = atomicTerm();
				expect(',');
				auto b = atomicTerm();
				expect(')');
				return comp(Tag::or1, comp(Tag::eq, b, a), comp(Tag::lt, b, a));
			}
			case s_is_int:
				return definedFunctor(Tag::isInt);
			case s_is_rat:
				return definedFunctor(Tag::isRat);
			case s_ite:
				err("$ite not supported", inappropriateError);
			case s_less:
				return definedFunctor(Tag::lt);
			case s_lesseq:
			{
				expect('(');
				auto a = atomicTerm();
				expect(',');
				auto b = atomicTerm();
				expect(')');
				return comp(Tag::or1, comp(Tag::eq, a, b), comp(Tag::lt, a, b));
			}
			case s_product:
				return definedFunctor(Tag::mul);
			case s_quotient:
				return definedFunctor(Tag::div);
			case s_quotient_e:
				return definedFunctor(Tag::divEuclid);
			case s_quotient_f:
				return definedFunctor(Tag::divFloor);
			case s_quotient_t:
				return definedFunctor(Tag::divTrunc);
			case s_remainder_e:
				return definedFunctor(Tag::remEuclid);
			case s_remainder_f:
				return definedFunctor(Tag::remFloor);
			case s_remainder_t:
				return definedFunctor(Tag::remTrunc);
			case s_round:
				return definedFunctor(Tag::round);
			case s_sum:
				return definedFunctor(Tag::add);
			case s_to_int:
				return definedFunctor(Tag::toInt);
			case s_to_rat:
				return definedFunctor(Tag::toRat);
			case s_to_real:
				return definedFunctor(Tag::toReal);
			case s_true:
				return bools + 1;
			case s_truncate:
				return definedFunctor(Tag::trunc);
			case s_uminus:
				return definedFunctor(Tag::minus);
			}
			break;
		}
		case k_num:
			return num1;
		case k_var:
		{
			for (auto i = vars.rbegin(), e = vars.rend(); i != e; ++i)
				if (i->first == s) return i->second;
			if (!cnfMode) err("unknown variable");
			auto a = var(vars.n, &tindividual);
			vars.add(make_pair(s, a));
			return a;
		}
		case k_word:
		{
			auto a = fn(s, 0);

			// Not a function call
			if (tok != '(') return a;

			// Function is being called, so gather the function and arguments
			Vec<Expr*> v(1, a);
			args(v);
			return comp(Tag::call, v);
		}
		}
		err("expected expression");
	}

	Expr* infixUnary() {
		auto a = atomicTerm();
		switch (tok) {
		case '=':
			lex();
			return comp(Tag::eq, a, atomicTerm());
		case k_ne:
			lex();
			return comp(Tag::not1, comp(Tag::eq, a, atomicTerm()));
		}
		return a;
	}

	Expr* quant(Tag tag) {
		lex();
		expect('[');
		Vec<Expr*> v(1);
		auto o = vars.n;
		do {
			if (tok != k_var) err("expected variable");
			auto s = str;
			lex();
			auto ty = &tindividual;
			if (eat(':')) {
				ty = atomicType();
				if (ty == &tbool) err("$o is not a valid variable type", inappropriateError);
			}
			auto a = var(vars.n, ty);
			vars.add(make_pair(s, a));
			v.add(a);
		} while (eat(','));
		expect(']');
		expect(':');
		v[0] = unary();
		vars.n = o;
		return comp(tag, v);
	}

	Expr* unary() {
		switch (tok) {
		case '!':
			return quant(Tag::all);
		case '(':
		{
			lex();
			auto a = logicFormula();
			expect(')');
			return a;
		}
		case '?':
			return quant(Tag::exists);
		case '~':
			lex();
			return comp(Tag::not1, unary());
		}
		return infixUnary();
	}

	Expr* associativeLogicFormula(Tag tag, Expr* a) {
		Vec<Expr*> v(1, a);
		auto k = tok;
		while (eat(k)) v.add(unary());
		return comp(tag, v);
	}

	Expr* logicFormula() {
		auto a = unary();
		switch (tok) {
		case '&':
			return associativeLogicFormula(Tag::and1, a);
		case '|':
			return associativeLogicFormula(Tag::or1, a);
		case k_eqv:
			lex();
			return comp(Tag::eqv, a, unary());
		case k_imp:
			lex();
			return imp(a, unary());
		case k_impr:
			lex();
			return imp(unary(), a);
		case k_nand:
			lex();
			return comp(Tag::not1, comp(Tag::and1, a, unary()));
		case k_nor:
			lex();
			return comp(Tag::not1, comp(Tag::or1, a, unary()));
		case k_xor:
			lex();
			return comp(Tag::not1, comp(Tag::eqv, a, unary()));
		}
		return a;
	}

	// Top level
	Str* wordOrDigits() {
		switch (tok) {
		case k_num:
		{
			auto r = intern(srck, src - srck);
			lex();
			return r;
		}
		case k_word:
		{
			auto r = str;
			lex();
			return r;
		}
		}
		err("expected name");
	}

	void ignore() {
		switch (tok) {
		case '(':
			lex();
			while (!eat(')')) ignore();
			return;
		case 0:
			err("too many '('s");
		}
		lex();
	}

	Parser1(const char* file, const Select& select): Parser(file), select(select) {
		lex();
		while (tok) {
			// SORT
			assert(!vars.n);
			bufp = buf;
			auto kw = wordOrDigits() - keywords;
			///
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
				size_t parens = 0;
				while (eat('(')) ++parens;
				cnfMode = 1;
				neg.n = pos.n = 0;
				do {
					auto no = eat('~');
					auto parens = eat('(');
					auto a = infixUnary();
					if (parens) expect(')');
					if (a->tag == Tag::not1) {
						a = at(a, 0);
						no = no ^ 1;
					}
					typing(a, &tbool);
					(no ? neg : pos).add(a);
				} while (eat('|'));
				while (parens--) expect(')');
				vars.n = 0;

				// Select
				if (select.count(name)) clause();
				break;
			}
			case s_fof:
			case s_tcf:
			case s_tff:
			{
				expect(',');

				// Role
				auto role = wordOrDigits() - keywords;
				expect(',');

				// Type
				if (role == s_type) {
					size_t parens = 0;
					while (eat('(')) ++parens;

					auto s = wordOrDigits();
					expect(':');
					if (eatDollarWord(s_tType)) {
						if (eat('>')) err("type constructors not supported", inappropriateError);

						// The symbol will be used as the name of a type. No particular action is required at this point, so accept
						// this and move on.
					} else {
						// The symbol is the name of a function with the specified type. Make sure it has this type.
						fn(s, topLevelType());
					}

					while (parens--) expect(')');
					break;
				}

				// Formula
				cnfMode = 0;
				auto a = logicFormula();
				assert(!vars.n);
				typing(a, &tbool);

				// Select
				if (!select.count(name)) break;

				// Conjecture
				if (role == s_conjecture) {
					// If multiple conjectures occur in a problem, there are two possible interpretations (conjunction or
					// disjunction), and no consensus on which is correct, so rather than risk silently giving a wrong answer,
					// reject the problem as ambiguous and require it to be restated with the conjectures folded into one, using
					// explicit conjunction or disjunction
					static bool conjecture;
					if (conjecture) err("multiple conjectures not supported");
					a = comp(Tag::not1, a);
					conjecture = 1;
				}

				// Convert to clauses
				cnf(a);
				break;
			}
			case s_include:
			{
				auto dir = getenv("TPTP");
				if (!dir) err("TPTP environment variable not set");

				// File
				snprintf(buf, bufSize, "%s/%s", dir, name);
				auto file1 = strdup(buf);

				// Select and read
				if (eat(',')) {
					expect('[');

					Select sel(0);
					do {
						auto selName = wordOrDigits();
						if (select.count(selName->v)) sel.insert(selName->v);
					} while (eat(','));

					expect(']');
					Parser1 parser(file1, sel);
				} else {
					Parser1 parser(file1, select);
				}
				break;
			}
			case s_thf:
				err("thf not supported", inappropriateError);
			default:
				err("unknown language");
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

void tptp(const char* file) {
	Parser1 parser(file, Select(1));
}
