#include "main.h"

namespace {
enum
{
	k_distinctObj = parser_k,
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

// If a term does not already have a type, assign it a specified one.
void defaultType(term a, type rty) {
	// A statement about the return type of a function call, can directly imply the type of the function. This generally does not
	// apply to basic operators; in most cases, they already have a definite type. That is not entirely true of the arithmetic
	// operators, but we don't try to do global type inference to figure those out.
	if (tag(a) != tag::Fn) return;

	// This is only a default assignment, only relevant if the function does not already have a type.
	auto p = a.getAtom();
	if (p->ty == kind::Unknown) p->ty = ftype(rty, a.begin(), a.end());
}

struct selection: set<const char*> {
	bool all;

	explicit selection(bool all): all(all) {
	}

	size_t count(const char* s) const {
		if (all) return 1;
		return set<const char*>::count(s);
	}
};

struct parser1: parser {
	// SORT
	bool cnfMode;
	Problem& problem;
	const selection& sel;
	vec<pair<string*, term>> vars;
	///

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

	// Types.
	type atomicType() {
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
				return kind::Individual;
			case s_int:
				return kind::Integer;
			case s_o:
				return kind::Bool;
			case s_rat:
				return kind::Rational;
			case s_real:
				return kind::Real;
			}
			break;
		case k_id:
			return type(s);
		}
		err("Expected type");
	}

	type topLevelType() {
		if (eat('(')) {
			vec<type> v(1);
			do v.push_back(atomicType());
			while (eat('*'));
			expect(')');
			expect('>');
			v[0] = atomicType();
			return type(kind::Fn, v);
		}
		auto ty = atomicType();
		if (eat('>')) return type(kind::Fn, atomicType(), ty);
		return ty;
	}

	// Terms.
	void args(vec<term>& v) {
		expect('(');
		do v.push_back(atomicTerm());
		while (eat(','));
		expect(')');
	}

	term definedFunctor(tag t) {
		vec<term> v(1, t);
		args(v);
		return term(v);
	}

	term atomicTerm() {
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
			vec<term> v;
			switch (keyword(s)) {
			case s_ceiling:
				return definedFunctor(tag::Ceil);
			case s_difference:
				return definedFunctor(tag::Sub);
			case s_distinct:
			{
				args(v);
				for (auto& a: v) defaultType(a, kind::Individual);
				vec<term> inequalities(1, tag::And);
				for (auto i = v.begin(), e = v.end(); i != e; ++i)
					for (auto j = v.begin(); j != i; ++j) inequalities.push_back(term(tag::Not, term(tag::Eq, *i, *j)));
				return term(inequalities);
			}
			case s_false:
				return tag::False;
			case s_floor:
				return definedFunctor(tag::Floor);
			case s_greater:
				args(v);
				return term(tag::Lt, v[1], v[0]);
			case s_greatereq:
				args(v);
				return term(tag::Le, v[1], v[0]);
			case s_is_int:
				return definedFunctor(tag::IsInteger);
			case s_is_rat:
				return definedFunctor(tag::IsRational);
			case s_less:
				return definedFunctor(tag::Lt);
			case s_lesseq:
				return definedFunctor(tag::Le);
			case s_product:
				return definedFunctor(tag::Mul);
			case s_quotient:
				return definedFunctor(tag::Div);
			case s_quotient_e:
				return definedFunctor(tag::DivE);
			case s_quotient_f:
				return definedFunctor(tag::DivF);
			case s_quotient_t:
				return definedFunctor(tag::DivT);
			case s_remainder_e:
				return definedFunctor(tag::RemE);
			case s_remainder_f:
				return definedFunctor(tag::RemF);
			case s_remainder_t:
				return definedFunctor(tag::RemT);
			case s_round:
				return definedFunctor(tag::Round);
			case s_sum:
				return definedFunctor(tag::Add);
			case s_to_int:
				return definedFunctor(tag::ToInteger);
			case s_to_rat:
				return definedFunctor(tag::ToRational);
			case s_to_real:
				return definedFunctor(tag::ToReal);
			case s_true:
				return tag::True;
			case s_truncate:
				return definedFunctor(tag::Trunc);
			case s_uminus:
				return definedFunctor(tag::Neg);
			}
			break;
		}
		case k_id:
		{
			term a(s, kind::Unknown);

			// Not a function call.
			if (tok != '(') return a;

			// Function is being called, so gather the function and arguments.
			vec<term> v(1, a);
			args(v);

			// By the TPTP specification, symbols can be assumed Boolean or individual, if not previously specified otherwise.
			// First-order logic does not allow functions to take Boolean arguments, so the arguments can default to individual. But
			// we cannot yet make any assumption about the function return type. For all we know here, it could still be Boolean.
			// Leave it to the caller, which will know from context whether that is the case.
			for (size_t i = 1; i != v.size(); ++i) defaultType(v[i], kind::Individual);

			return term(v);
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
			for (auto i = vars.rbegin(), e = vars.rend(); i != e; ++i)
				if (i->first == s) return i->second;
			if (!cnfMode) err("Unknown variable");
			auto x = var(vars.size(), kind::Individual);
			vars.push_back(make_pair(s, x));
			return x;
		}
		}
		err("Expected term");
	}

	term infixUnary() {
		auto a = atomicTerm();
		switch (tok) {
		case '=':
		{
			lex();
			auto b = atomicTerm();
			defaultType(a, kind::Individual);
			defaultType(b, kind::Individual);
			return term(tag::Eq, a, b);
		}
		case k_ne:
		{
			lex();
			auto b = atomicTerm();
			defaultType(a, kind::Individual);
			defaultType(b, kind::Individual);
			return term(tag::Not, term(tag::Eq, a, b));
		}
		}
		defaultType(a, kind::Bool);
		return a;
	}

	term quant(tag t) {
		lex();
		expect('[');
		auto old = vars.size();
		// TODO: check generated code
		vec<term> v{t, tag::False};
		do {
			if (tok != k_var) err("Expected variable");
			auto s = str;
			lex();
			type ty = kind::Individual;
			if (eat(':')) ty = atomicType();
			auto x = var(vars.size(), ty);
			vars.push_back(make_pair(s, x));
			v.push_back(x);
		} while (eat(','));
		expect(']');
		expect(':');
		v[1] = unary();
		vars.resize(old);
		return term(v);
	}

	term unary() {
		switch (tok) {
		case '!':
			return quant(tag::All);
		case '(':
		{
			lex();
			auto a = logicFormula();
			expect(')');
			return a;
		}
		case '?':
			return quant(tag::Exists);
		case '~':
			lex();
			return term(tag::Not, unary());
		}
		return infixUnary();
	}

	term associativeLogicFormula(tag t, term a) {
		vec<term> v{t, a};
		auto k = tok;
		while (eat(k)) v.push_back(unary());
		return term(v);
	}

	term logicFormula() {
		auto a = unary();
		switch (tok) {
		case '&':
			return associativeLogicFormula(tag::And, a);
		case '|':
			return associativeLogicFormula(tag::Or, a);
		case k_eqv:
			lex();
			return term(tag::Eqv, a, unary());
		case k_imp:
			lex();
			return imp(a, unary());
		case k_impr:
			lex();
			return imp(unary(), a);
		case k_nand:
			lex();
			return term(tag::Not, term(tag::And, a, unary()));
		case k_nor:
			lex();
			return term(tag::Not, term(tag::Or, a, unary()));
		case k_xor:
			lex();
			return term(tag::Not, term(tag::Eqv, a, unary()));
		}
		return a;
	}

	// Top level.
	string* wordOrDigits() {
		switch (tok) {
		case k_id:
		{
			auto r = str;
			lex();
			return r;
		}
		case k_integer:
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

	parser1(const char* file, const selection& sel, Problem& problem): parser(file), sel(sel), problem(problem) {
		lex();
		while (tok) {
			vars.clear();
			auto kw = keyword(wordOrDigits());
			expect('(');
			auto name = wordOrDigits()->v;
			switch (kw) {
			case s_cnf:
			{
				expect(',');

				// Role.
				wordOrDigits();
				expect(',');

				// Literals.
				cnfMode = 1;
				auto a = quantify(logicFormula());

				// Select.
				if (!sel.count(name)) break;

				// Clause.
				problem.axiom(a, file, name);
				break;
			}
			case s_fof:
			case s_tff:
			{
				expect(',');

				// Role.
				auto role = keyword(wordOrDigits());
				expect(',');

				// Type.
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
						term _(s, topLevelType());
					}

					while (parens--) expect(')');
					break;
				}

				// Formula.
				cnfMode = 0;
				auto a = logicFormula();
				assert(vars.empty());
				check(a, kind::Bool);

				// Select.
				if (!sel.count(name)) break;

				// Conjecture.
				if (role == s_conjecture) {
					problem.conjecture(a, file, name);
					break;
				}

				// Ordinary formula.
				problem.axiom(a, file, name);
				break;
			}
			case s_include:
			{
				auto dir = getenv("TPTP");
				if (!dir) err("TPTP environment variable not set");

				// File.
				snprintf(buf, sizeof buf, "%s/%s", dir, name);
				auto file1 = intern(buf)->v;

				// Select and read.
				if (eat(',')) {
					expect('[');

					selection sel1(0);
					do {
						auto selName = wordOrDigits();
						if (sel.count(selName->v)) sel1.add(selName->v);
					} while (eat(','));

					expect(']');
					parser1 _(file1, sel1, problem);
				} else {
					parser1 _(file1, sel, problem);
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

void parseTptp(const char* file, Problem& problem) {
	parser1 _(file, selection(1), problem);
}

namespace {
// When assigning numbers to anonymous clauses, we need to check which numbers have already been used. To this end, parse strings to
// numbers, harmlessly returning zero if the string is not a number within bounds.
size_t parseId(const char* s) {
	size_t n = 0;
	do {
		auto c = *s++;
		if (!isDigit(c)) return 0;
		auto n1 = n * 10 + c - '0';
		if (n1 < n) return 0;
		n = n1;
	} while (*s);
	return n;
}

// Like most computer languages, TPTP has the notion of a normal name beginning with a letter or underscore, and containing those
// characters or digits.
bool normal(const char* s) {
	if (!*s) return 0;
	if (isDigit(*s)) return 0;
	while (*s)
		if (!isWord(*s++)) return 0;
	return 1;
}

// Formulas have the additional rule that all-digits counts as a normal name.
bool allDigits(const char* s) {
	if (!*s) return 0;
	while (*s)
		if (!isDigit(*s++)) return 0;
	return 1;
}

// Unlike some computer languages, TPTP allows other names to be used if quoted.
void quote(int q, const char* s) {
	putchar(q);
	while (*s) {
		if (*s == q || *s == '\\') putchar('\\');
		putchar(*s++);
	}
	putchar(q);
}

// Print the name of a formula, taking into account that it may have an actual name or just an ID number, and in the former case it
// may or may not require quotes.
void prname(const map<uint32_t, uint32_t>& ids, size_t o) {
	auto s = getName((AbstractFormula*)heap->ptr(o));
	if (!s) {
		print(ids.at(o));
		return;
	}
	if (normal(s) || allDigits(s)) {
		print(s);
		return;
	}
	quote('\'', s);
}

// Ordinary names have the same consideration about possibly requiring quotes, but don't have the special cases for ID numbers and
// strings of digits.
void prname(const char* s) {
	if (normal(s)) {
		print(s);
		return;
	}
	quote('\'', s);
}

// Only some types will occur in proofs.
void pr(type ty) {
	switch (kind(ty)) {
	case kind::Individual:
		print("$i");
		return;
	case kind::Integer:
		print("$int");
		return;
	case kind::Rational:
		print("$rat");
		return;
	case kind::Real:
		print("$real");
		return;
	case kind::Sym:
	{
		auto s = typePtr(ty.offset)->s;
		prname(s);
		return;
	}
	}
	unreachable;
}

void pr(term a, term parent = tag::False);

void dfunctor(const char* op, term a) {
	print(op);
	putchar('(');
	for (size_t i = 1; i != a.size(); ++i) {
		if (i) putchar(',');
		pr(a[i]);
	}
	putchar(')');
}

void quant(char op, term a) {
	putchar(op);
	putchar('[');
	joining;
	for (size_t i = 2; i != a.size(); ++i) {
		join(',');
		auto x = a[i];
		pr(x);
		auto ty = type(x);
		if (ty != kind::Individual) {
			putchar(':');
			pr(ty);
		}
	}
	print("]:");
	pr(a[1], a);
}

// Infix connectives may need to be surrounded by parentheses to disambiguate, depending on what the parent term was.
bool needParens(term a, term parent) {
	// CNF conversion may sometimes generate conjunctions or disjunctions with only one operand. In that case, the operator will not
	// actually occur in the printed form, so the requirement for parentheses cannot arise.
	if (a.size() == 1) return 0;

	// Otherwise, only some parent terms have the potential to interfere with operator parsing.
	switch (tag(parent)) {
	case tag::All:
	case tag::And:
	case tag::Eqv:
	case tag::Exists:
	case tag::Not:
	case tag::Or:
		return 1;
	}
	return 0;
}

void infixConnective(const char* op, term a, term parent) {
	assert(a.size());
	auto p = needParens(a, parent);
	if (p) putchar('(');
	joining;
	for (auto b: a) {
		join(op);
		pr(b, a);
	}
	if (p) putchar(')');
}

size_t sknames;

void pr(term a, term parent) {
	switch (tag(a)) {
	case tag::Add:
		dfunctor("$sum", a);
		return;
	case tag::All:
		quant('!', a);
		return;
	case tag::And:
		infixConnective(" & ", a, parent);
		return;
	case tag::Ceil:
		dfunctor("$ceiling", a);
		return;
	case tag::DistinctObj:
		quote('"', a.getAtom()->s);
		return;
	case tag::Div:
		dfunctor("$quotient", a);
		return;
	case tag::DivE:
		dfunctor("$quotient_e", a);
		return;
	case tag::DivF:
		dfunctor("$quotient_f", a);
		return;
	case tag::DivT:
		dfunctor("$quotient_t", a);
		return;
	case tag::Eq:
		pr(a[1]);
		putchar('=');
		pr(a[2]);
		return;
	case tag::Eqv:
		infixConnective(" <=> ", a, parent);
		return;
	case tag::Exists:
		quant('?', a);
		return;
	case tag::False:
		print("$false");
		return;
	case tag::Floor:
		dfunctor("$floor", a);
		return;
	case tag::Fn:
	{
		auto p = a.getAtom();
		if (!p->s) {
			string* s;
			do {
				sprintf(buf, "sK%zu", ++sknames);
				s = intern(buf);
			} while (s->sym);
			// TODO: do this more efficiently
			s->sym = a[0].getAtomOffset();
			p->s = s->v;
		}
		prname(p->s);
		// TODO: do this more efficiently
		if (a.size() == 1) return;
		putchar('(');
		joining;
		for (auto b: a) {
			join(',');
			pr(b);
		}
		putchar(')');
		return;
	}
	case tag::Integer:
		mpz_out_str(stdout, 10, a.mpz());
		return;
	case tag::IsInteger:
		dfunctor("$is_int", a);
		return;
	case tag::IsRational:
		dfunctor("$is_rat", a);
		return;
	case tag::Le:
		dfunctor("$lesseq", a);
		return;
	case tag::Lt:
		dfunctor("$less", a);
		return;
	case tag::Mul:
		dfunctor("$product", a);
		return;
	case tag::Neg:
		dfunctor("$uminus", a);
		return;
	case tag::Not:
		putchar('~');
		pr(a[1], a);
		return;
	case tag::Or:
		infixConnective(" | ", a, parent);
		return;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpq_out_str(stdout, 10, a1);
		if (!mpz_cmp_ui(mpq_denref(a1), 1)) printf("/1");
		return;
	}
	case tag::RemE:
		dfunctor("$remainder_e", a);
		return;
	case tag::RemF:
		dfunctor("$remainder_f", a);
		return;
	case tag::RemT:
		dfunctor("$remainder_t", a);
		return;
	case tag::Round:
		dfunctor("$round", a);
		return;
	case tag::Sub:
		dfunctor("$difference", a);
		return;
	case tag::ToInteger:
		dfunctor("$to_int", a);
		return;
	case tag::ToRational:
		dfunctor("$to_rat", a);
		return;
	case tag::ToReal:
		dfunctor("$to_real", a);
		return;
	case tag::True:
		print("$true");
		return;
	case tag::Trunc:
		dfunctor("$truncate", a);
		return;
	case tag::Var:
	{
		auto i = a.varIdx();
		if (i < 26) putchar('A' + i);
		else
			printf("Z%zu", i - 25);
		return;
	}
	}
	unreachable;
}

void prliterals(const clause& c) {
	joining;
	for (auto a: c.first) {
		join(" | ");
		if (tag(a) == tag::Eq) {
			pr(a[1]);
			print("!=");
			pr(a[2]);
			continue;
		}
		putchar('~');
		pr(a);
	}
	for (auto a: c.second) {
		join(" | ");
		pr(a);
	}
	if (c.first.size() + c.second.size() == 0) print("$false");
}
} // namespace

void tptpClause(const clause& c, size_t id) {
	printf("cnf(%zu, plain, ", id);
	prliterals(c);
	print(").\n");
}

void tptpProof(const vec<uint32_t>& proofv) {
	// We will need to assign identity numbers to all formulas that don't already have names. In preparation for that, find out
	// which numbers have already been used as names.
	set<size_t> used;
	for (auto o: proofv) {
		auto f = (AbstractFormula*)heap->ptr(o);
		auto s = getName(f);
		if (!s) continue;
		auto i = parseId(s);
		if (i) used.add(i);
	}

	// Assign identity numbers to all formulas that don't already have names.
	size_t i = 1;
	map<uint32_t, uint32_t> ids;
	for (auto o: proofv) {
		auto f = (AbstractFormula*)heap->ptr(o);
		if (getName(f)) continue;
		while (used.count(i)) ++i;
		ids.add(o, i++);
	}

	// Will also need to give names to skolem functions that up to now have been anonymous. (It's efficient to defer this until
	// printing the proof, because some skolem functions may not feature in the proof, so need never be named.)
	sknames = 0;

	// Print the proof.
	for (auto o: proofv) {
		auto f0 = (AbstractFormula*)heap->ptr(o);
		switch (f0->Class) {
		case FormulaClass::Axiom:
		case FormulaClass::Conjecture:
		{
			auto f = (InputFormula*)f0;
			print("tff(");
			prname(ids, o);
			print(f->Class == FormulaClass::Axiom ? ", axiom, " : ", conjecture, ");
			pr(f->a);

			// Axioms and conjectures are input formulas.
			print(", file(");
			quote('\'', basename(f->file));
			putchar(',');
			prname(ids, o);
			break;
		}
		case FormulaClass::Clause:
		{
			auto f = (Clause*)f0;
			print("cnf(");
			prname(ids, o);
			print(", plain, ");
			prliterals(f->c);

			// Every clause must be either converted from a formula, or inferred from other clauses. When e.g. definitions are
			// introduced, they are always introduced as formulas first, then converted to clauses.
			assert(f->from);

			// If a clause was not inferred from other clauses, then it must have been converted from a formula. In general, CNF
			// conversion produces a clause set that is not exactly equivalent to the original formula (because new symbols may be
			// introduced), but is equisatisfiable.
			printf(", inference(%s,[status(%s)],[", ruleNames[(int)f->rl], f->rl == rule::cnf ? "esa" : "thm");

			// It is possible to implement more complex inference rules that infer a clause from several source clauses, but at the
			// moment, the largest number of source clauses is 2.
			prname(ids, f->from);
			if (f->from1) {
				putchar(',');
				prname(ids, f->from1);
			}
			putchar(']');
			break;
		}
		case FormulaClass::Definition:
		{
			auto f = (Formula*)f0;
			print("tff(");
			prname(ids, o);
			print(", definition, ");
			pr(f->a);

			// Definitions are introduced during CNF conversion. They are the only formulas with no source, so printing their source
			// record is simple.
			print(", introduced(definition");
			break;
		}
		case FormulaClass::Negation:
		{
			auto f = (NegatedFormula*)f0;
			print("tff(");
			prname(ids, o);
			print(", negated_conjecture, ");
			pr(f->a);

			// The negated conjecture is not an input formula, but is an initial formula. It is derived straightforwardly from the
			// conjecture.
			print(", inference(negate,[status(ceq)],[");
			prname(ids, f->from);
			putchar(']');
			break;
		}
		}
		print(")).\n");
	}
}
