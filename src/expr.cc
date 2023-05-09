#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// Numbers need to be interned, to preserve the property that equal terms have pointer equality

// TODO: write test problems for integer division
// Integers
struct IntegerCmp {
	static bool eq(Tag tag, mpz_t a, size_t n, Integer* b) {
		return mpz_cmp(a, b->val) == 0;
	}
	static bool eq(Integer* a, Integer* b) {
		return mpz_cmp(a->val, b->val) == 0;
	}

	static size_t hash(Tag tag, mpz_t a, size_t n) {
		return mpz_get_ui(a);
	}
	static size_t hash(Integer* a) {
		return mpz_get_ui(((Integer*)a)->val);
	}
};

void clear(mpz_t val) {
	mpz_clear(val);
}

static Set<Tag, mpz_t, Integer, IntegerCmp> integers;

Integer* integer(mpz_t val) {
	return integers.intern(Tag::integer, val, 0);
}

// Rationals
bool eq(Tag tag, mpq_t a, size_t n, Rational* b) {
	return mpq_equal(a, b->val);
}

size_t hash(mpq_t a) {
	return hashCombine(mpz_get_ui(mpq_numref(a)), mpz_get_ui(mpq_denref(a)));
}

struct RationalCmp {};

void clear(mpq_t val) {
	mpq_clear(val);
}

static Set<Tag, mpq_t, Rational, RationalCmp> rationals;

Rational* rational(Tag tag, mpq_t val) {
	return rationals.intern(tag, val, 0);
}

// Composite expressions
struct CompCmp {
	static bool eq(Tag tag, Expr** a, size_t n, Comp* b) {
		return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
	}
	static bool eq(Comp* a, Comp* b) {
		return eq(a->tag, a->v, a->n, b);
	}

	static size_t hash(Tag tag, Expr** a, size_t n) {
		// TODO: hashCombine?
		return fnv(a, n * sizeof *a);
	}
	static size_t hash(Comp* a) {
		return hash(a->tag, a->v, a->n);
	}
};

static void clear(Expr** a) {
}

static Set<Tag, Expr**, Comp, CompCmp> comps;

// TODO: static
bool constant(Expr* a) {
	switch (a->tag) {
	case Tag::distinctObj:
	case Tag::integer:
	case Tag::rational:
	case Tag::real:
		return 1;
	}
	return 0;
}

Expr* comp(Tag tag, Expr** v, size_t n) {
	switch (tag) {
	case Tag::add:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) {
			auto tag = x->tag;
			assert(tag == y->tag);
			if (tag == Tag::integer) {
				mpz_t r;
				mpz_init(r);
				mpz_add(r, ((Integer*)x)->val, ((Integer*)y)->val);
				return integer(r);
			}
			mpq_t r;
			mpq_init(r);
			mpq_add(r, ((Rational*)x)->val, ((Rational*)y)->val);
			return rational(tag, r);
		}
		break;
	}
	}
	return comps.intern(tag, v, n);
}

Expr* expr(Tag tag, Expr* a, Expr* b) {
	static Expr* v[2];
	v[0] = a;
	v[1] = b;
	return comp(tag, v, 2);
}

Expr* expr(Tag tag, const Vec<Expr*>& v) {
	assert(v.size());
	return comps.intern(tag, v.data, v.n);
}

Type* type(Expr* a) {
	switch (a->tag) {
	case Tag::add:
	case Tag::ceil:
	case Tag::div:
	case Tag::divEuclid:
	case Tag::divFloor:
	case Tag::divTrunc:
	case Tag::floor:
	case Tag::minus:
	case Tag::mul:
	case Tag::remEuclid:
	case Tag::remFloor:
	case Tag::remTrunc:
	case Tag::round:
	case Tag::sub:
	case Tag::trunc:
		return type(at(a, 0));
	case Tag::all:
	case Tag::and1:
	case Tag::eq:
	case Tag::eqv:
	case Tag::exists:
	case Tag::false1:
	case Tag::isInteger:
	case Tag::isRational:
	case Tag::le:
	case Tag::lt:
	case Tag::not1:
	case Tag::or1:
	case Tag::true1:
		return &tbool;
	case Tag::call:
	{
		auto f = (Fn*)at(a, 0);
		assert(f->tag == Tag::fn);
		return at(f->ty, 0);
	}
	case Tag::distinctObj:
		return &tindividual;
	case Tag::fn:
		return ((Fn*)a)->ty;
	case Tag::integer:
	case Tag::toInteger:
		return &tinteger;
	case Tag::rational:
	case Tag::toRational:
		return &trational;
	case Tag::real:
	case Tag::toReal:
		return &treal;
	case Tag::var:
		return ((Var*)a)->ty;
	}
	unreachable;
}

Type* ftype(Type* rty, Expr** first, Expr** last) {
	if (first == last) return rty;
	Vec<Type*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return type(v);
}

int cmp(Expr* a, Expr* b) {
	assert(a->tag == b->tag);
	if (a == b) return 0;
	switch (a->tag) {
	case Tag::integer:
		return mpz_cmp(((Integer*)a)->val, ((Integer*)b)->val);
	case Tag::rational:
	case Tag::real:
		return mpq_cmp(((Rational*)a)->val, ((Rational*)b)->val);
	}
	unreachable;
}
