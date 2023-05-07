#include "all.h"

Ex tbool = {Bool};
Ex tinteger = {Integer};
Ex trational = {Rational};
Ex treal = {Real};
Ex tindividual = {Individual};

Ex bools[2] = {{False}, {True}};

// TODO: rename?
Ex* gensym(Ex* ty) {
	auto a = (Ex*)malloc(offsetof(Ex, s) + sizeof(char*));
	a->tag = Fn;
	a->ty = ty;
	a->s = 0;
	return a;
}

// Composite expressions
bool eq(int tag, const Ex** a, size_t n, const Ex* b) {
	return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
}

struct CompCmp {};

static set<Ex**, Ex, CompCmp> comps;

Ex* ex(int tag, Ex* a, Ex* b) {
	static Ex* v[2];
	v[0] = a;
	v[1] = b;
	return comps.intern(tag, v, 2);
}

Ex* ex(int tag, const vec<Ex*>& v) {
	assert(v.size());
	return comps.intern(tag, v.data, v.n);
}

Ex* type(const Ex* a) {
	switch (a->tag) {
	case Add:
	case Ceil:
	case Div:
	case DivE:
	case DivF:
	case DivT:
	case Floor:
	case Mul:
	case Neg:
	case RemE:
	case RemF:
	case RemT:
	case Round:
	case Sub:
	case Trunc:
		return type(at(a, 0));
	case All:
	case And:
	case Eq:
	case Eqv:
	case Exists:
	case False:
	case IsInteger:
	case IsRational:
	case Le:
	case Lt:
	case Not:
	case Or:
	case True:
		return &tbool;
	case Call:
		a = at(a, 0);
		assert(a->tag == Fn);
		return at(a->ty, 0);
	case DistinctObj:
		return &tindividual;
	case Fn:
	case Var:
		return a->ty;
	case Integer:
	case ToInteger:
		return &tinteger;
	case Rational:
	case ToRational:
		return &trational;
	case Real:
	case ToReal:
		return &treal;
	}
	unreachable;
}

Ex* ftype(const Ex* rty, const Ex** first, const Ex** last) {
	if (first == last) return rty;
	vec<Ex*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return ex(Call, v);
}

// TODO: eliminate this?
int cmp(ex a, ex b) {
	// Fast test for equality
	if (a == b) return 0;

	// If the tags differ, just sort in tag order; not meaningful, but it doesn't have to be meaningful, just consistent
	if (a->tag != b->tag) return (int)a->tag - (int)b->tag;

	// Numbers sort in numerical order
	switch (a->tag) {
	case Integer:
		return mpz_cmp(a.mpz(), b.mpz());
	case Rational:
		return mpq_cmp(a.mpq(), b.mpq());
	}

	// Compound terms sort in lexicographic order
	auto an = a.size();
	auto bn = b.size();
	auto n = min(an, bn);
	for (size_t i = 0; i < n; ++i) {
		auto c = cmp(at(a, i), b[i]);
		if (c) return c;
	}
	if (an - bn) return an - bn;

	// They are different terms with the same tags and no different components, so they must be different atoms; just do a straight
	// binary comparison
	return memcmp(&a, &b, sizeof a);
}

static void check(ex a, size_t arity) {
	if (a.size() - 1 == arity) return;
	sprintf(buf, "Expected %zu args, received %zu", arity, a.size() - 1);
	err(buf);
}

void check(ex a, type ty) {
	// In first-order logic, a function cannot return a function, nor can a variable store one. (That would be higher-order logic.)
	// The code should be written so that neither the top-level callers nor the recursive calls, can ever ask for a function to be
	// returned.
	assert(kind(ty) != kind::Fn);

	// All symbols used in a formula must have specified types by the time this check is run. Otherwise, there would be no way of
	// knowing whether the types they will be given in the future, would have passed the check.
	if (ty == kind::Unknown) err("Unspecified type");

	// Need to handle calls before checking the type of this term, because the type of a call is only well-defined if the type of
	// the function is well-defined
	if (a->tag == Fn && a.size() > 1) {
		auto fty = a.getAtom()->ty;
		if (kind(fty) != kind::Fn) err("Called a non-function");
		check(a, fty.size() - 1);
		if (ty != fty[0]) err("Type mismatch");
		for (size_t i = 0; i < a->n; ++i) {
			switch (kind(fty[i])) {
			case kind::Bool:
			case kind::Fn:
				err("Invalid type for function argument");
			}
			check(at(a, i), fty[i]);
		}
		return;
	}

	// The core of the check: Make sure the term is of the required type
	if (type(a) != ty) err("Type mismatch");

	// Further checks can be done depending on operator. For example, arithmetic operators should have matching numeric arguments.
	switch (a->tag) {
	case Add:
	case DivE:
	case DivF:
	case DivT:
	case Mul:
	case RemE:
	case RemF:
	case RemT:
	case Sub:
		check(a, 2);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for arithmetic");
		}
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), ty);
		return;
	case All:
	case Exists:
		check(at(a, 0), kind::Bool);
		return;
	case And:
	case Eqv:
	case Not:
	case Or:
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), kind::Bool);
		return;
	case Ceil:
	case Floor:
	case IsInteger:
	case IsRational:
	case Neg:
	case Round:
	case ToInteger:
	case ToRational:
	case ToReal:
	case Trunc:
		check(a, 1);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for arithmetic");
		}
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), ty);
		return;
	case DistinctObj:
	case False:
	case Fn:
	case Integer:
	case Rational:
	case True:
		return;
	case Div:
		check(a, 2);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for rational division");
		}
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), ty);
		return;
	case Eq:
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Bool:
		case kind::Fn:
			err("Invalid type for equality");
		}
		check(at(a, 0), ty);
		check(at(a, 1), ty);
		return;
	case Le:
	case Lt:
		check(a, 2);
		ty = type(at(a, 0));
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for comparison");
		}
		check(at(a, 0), ty);
		check(at(a, 1), ty);
		return;
	case Var:
		// A function would also be an invalid type for a variable, but we already checked for that
		if (kind(ty) == kind::Bool) err("Invalid type for variable");
		return;
	}
	unreachable;
}
