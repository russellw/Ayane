#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// Numbers need to be interned, to preserve the property that equal terms have pointer equality

// TODO: write test problems for integer division
// Integers
struct IntegerCmp {
	static bool eq(Tag tag, mpz_t val, size_t n, Integer* b) {
		return mpz_cmp(val, b->val) == 0;
	}
	static bool eq(Integer* a, Integer* b) {
		return mpz_cmp(a->val, b->val) == 0;
	}

	static size_t hash(Tag tag, mpz_t val, size_t n) {
		return mpz_get_ui(val);
	}
	static size_t hash(Integer* a) {
		return hash(Tag::integer, ((Integer*)a)->val, 0);
	}

	static Integer* make(Tag tag, mpz_t val, size_t n) {
		return new Integer(val);
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
struct RationalCmp {
	// TODO: can we now remove the Cmp class?
	static bool eq(Tag tag, mpq_t val, size_t n, Rational* b) {
		return mpq_equal(val, b->val);
	}
	static bool eq(Rational* a, Rational* b) {
		return mpq_equal(a->val, b->val);
	}

	static size_t hash(Tag tag, mpq_t val, size_t n) {
		return hashCombine(mpz_get_ui(mpq_numref(val)), mpz_get_ui(mpq_denref(val)));
	}
	static size_t hash(Rational* a) {
		return hash(a->tag, ((Rational*)a)->val, 0);
	}

	static Rational* make(Tag tag, mpq_t val, size_t n) {
		return new Rational(tag, val);
	}
};

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

	static Comp* make(Tag tag, Expr** val, size_t n) {
		auto p = malloc(offsetof(Comp, v) + n * sizeof(Expr*));
		auto a = new (p) Comp(tag, n);
		memcpy(a->v, val, n * sizeof(Expr*));
		return a;
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

Expr* op2(Expr* x, Expr* y, void (*fz)(mpz_t, const mpz_t, const mpz_t), void (*fq)(mpq_t, const mpq_t, const mpq_t)) {
	auto tag = x->tag;
	assert(tag == y->tag);

	if (tag == Tag::integer) {
		mpz_t r;
		mpz_init(r);
		fz(r, ((Integer*)x)->val, ((Integer*)y)->val);
		return integer(r);
	}

	mpq_t r;
	mpq_init(r);
	fq(r, ((Rational*)x)->val, ((Rational*)y)->val);
	return rational(tag, r);
}

Expr* div2(Expr* x, Expr* y, void (*fz)(mpz_t, const mpz_t, const mpz_t)) {
	auto tag = x->tag;
	assert(tag == y->tag);

	if (tag == Tag::integer) {
		mpz_t r;
		mpz_init(r);
		fz(r, ((Integer*)x)->val, ((Integer*)y)->val);
		return integer(r);
	}

	mpz_t xnum_yden;
	mpz_init(xnum_yden);
	mpz_mul(xnum_yden, mpq_numref(((Rational*)x)->val), mpq_denref(((Rational*)y)->val));

	mpz_t xden_ynum;
	mpz_init(xden_ynum);
	mpz_mul(xden_ynum, mpq_denref(((Rational*)x)->val), mpq_numref(((Rational*)y)->val));

	mpq_t r;
	mpq_init(r);
	fz(mpq_numref(r), xnum_yden, xden_ynum);

	mpz_clear(xnum_yden);
	mpz_clear(xden_ynum);
	return rational(tag, r);
}

Expr* comp(Tag tag, Expr** v, size_t n) {
	switch (tag) {
	case Tag::add:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_add, mpq_add);
		break;
	}
	case Tag::div:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, 0, mpq_div);
		break;
	}
	case Tag::divEuclid:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_ediv_q);
		break;
	}
	case Tag::divFloor:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_fdiv_q);
		break;
	}
	case Tag::divTrunc:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_tdiv_q);
		break;
	}
	case Tag::minus:
	{
		auto x = v[0];
		if (constant(x)) {
			auto tag = x->tag;
			if (tag == Tag::integer) {
				mpz_t r;
				mpz_init(r);
				mpz_neg(r, ((Integer*)x)->val);
				return integer(r);
			}
			mpq_t r;
			mpq_init(r);
			mpq_neg(r, ((Rational*)x)->val);
			return rational(tag, r);
		}
		break;
	}
	case Tag::mul:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_mul, mpq_mul);
		break;
	}
	case Tag::remEuclid:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_ediv_r);
		break;
	}
	case Tag::remFloor:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_fdiv_r);
		break;
	}
	case Tag::remTrunc:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_tdiv_r);
		break;
	}
	case Tag::sub:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_sub, mpq_sub);
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
