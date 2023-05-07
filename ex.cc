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
struct CompCmp {
	static bool eq(int tag, Ex** a, size_t n, Ex* b) {
		return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
	}
	static bool eq(Ex* a, Ex* b) {
		return eq(a->tag, a->v, a->n, b);
	}
	static size_t hash(int tag, Ex** a, size_t n) {
		// TODO: hashCombine?
		return fnv(a, n * sizeof *a);
	}
	static size_t hash(Ex* a) {
		return hash(a->tag, a->v, a->n);
	}
};
static void clear(Ex** a) {
}
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

Ex* type(Ex* a) {
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

Ex* ftype(Ex* rty, Ex** first, Ex** last) {
	if (first == last) return rty;
	vec<Ex*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return ex(Call, v);
}

int cmp(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	if (a == b) return 0;
	switch (a->tag) {
	case Integer:
		return mpz_cmp(a->mpz, b->mpz);
	case Rational:
	case Real:
		return mpq_cmp(a->mpq, b->mpq);
	}
	unreachable;
}
