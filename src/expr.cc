#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// Numbers need to be interned, to preserve the property that equal terms have pointer equality

// TODO: write test problems for integer division
// Integers
struct IntCmp {
	static bool eq(Tag tag, mpz_t val, size_t n, Int* b) {
		return mpz_cmp(val, b->val) == 0;
	}
	static bool eq(Int* a, Int* b) {
		return mpz_cmp(a->val, b->val) == 0;
	}

	static size_t hash(Tag tag, mpz_t val, size_t n) {
		return mpz_get_ui(val);
	}
	static size_t hash(Int* a) {
		return hash(Tag::integer, ((Int*)a)->val, 0);
	}

	static Int* make(Tag tag, mpz_t val, size_t n) {
		return new Int(val);
	}
};

void clear(mpz_t val) {
	mpz_clear(val);
}

static Set<Tag, mpz_t, Int, IntCmp> integers;

Int* integer(mpz_t val) {
	return integers.intern(Tag::integer, val, 0);
}

// Rationals
struct RatCmp {
	// TODO: can we now remove the Cmp class?
	static bool eq(Tag tag, mpq_t val, size_t n, Rat* b) {
		return tag == b->tag && mpq_equal(val, b->val);
	}
	static bool eq(Rat* a, Rat* b) {
		return a->tag == b->tag && mpq_equal(a->val, b->val);
	}

	static size_t hash(Tag tag, mpq_t val, size_t n) {
		return hashCombine(mpz_get_ui(mpq_numref(val)), mpz_get_ui(mpq_denref(val)));
	}
	static size_t hash(Rat* a) {
		return hash(a->tag, ((Rat*)a)->val, 0);
	}

	static Rat* make(Tag tag, mpq_t val, size_t n) {
		return new Rat(tag, val);
	}
};

void clear(mpq_t val) {
	mpq_clear(val);
}

static Set<Tag, mpq_t, Rat, RatCmp> rats;

Rat* rat(Tag tag, mpq_t val) {
	return rats.intern(tag, val, 0);
}

// Variables
Expr* var(size_t i, LeafType* ty) {
	// TODO: optimize
	while (ty->vars.n <= i) ty->vars.add(0);
	if (!ty->vars[i]) ty->vars[i] = new Var(ty);
	return ty->vars[i];
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
	case Tag::rat:
	case Tag::real:
		return 1;
	}
	return 0;
}

// Factor out common calculation patterns
Expr* op1(Expr* x, void (*f)(mpz_t, const mpz_t, const mpz_t)) {
	auto tag = x->tag;

	if (tag == Tag::integer) return x;

	auto x1 = (Rat*)x;

	mpq_t r;
	mpq_init(r);
	f(mpq_numref(r), mpq_numref(x1->val), mpq_denref(x1->val));
	return rat(tag, r);
}

Rat* toRat(Expr* x, Tag tag) {
	mpq_t r;
	mpq_init(r);
	if (x->tag == Tag::integer) mpz_set(mpq_numref(r), ((Int*)x)->val);
	else
		mpq_set(r, ((Rat*)x)->val);
	return rat(tag, r);
}

Expr* op2(Expr* x, Expr* y, void (*fz)(mpz_t, const mpz_t, const mpz_t), void (*fq)(mpq_t, const mpq_t, const mpq_t)) {
	auto tag = x->tag;
	assert(tag == y->tag);

	if (tag == Tag::integer) {
		mpz_t r;
		mpz_init(r);
		fz(r, ((Int*)x)->val, ((Int*)y)->val);
		return integer(r);
	}

	mpq_t r;
	mpq_init(r);
	fq(r, ((Rat*)x)->val, ((Rat*)y)->val);
	return rat(tag, r);
}

Expr* div2(Expr* x, Expr* y, void (*f)(mpz_t, const mpz_t, const mpz_t)) {
	auto tag = x->tag;
	assert(tag == y->tag);

	if (tag == Tag::integer) {
		mpz_t r;
		mpz_init(r);
		f(r, ((Int*)x)->val, ((Int*)y)->val);
		return integer(r);
	}

	auto x1 = (Rat*)x;
	auto y1 = (Rat*)y;

	mpz_t xnum_yden;
	mpz_init(xnum_yden);
	mpz_mul(xnum_yden, mpq_numref(x1->val), mpq_denref(y1->val));

	mpz_t xden_ynum;
	mpz_init(xden_ynum);
	mpz_mul(xden_ynum, mpq_denref(x1->val), mpq_numref(y1->val));

	mpq_t r;
	mpq_init(r);
	f(mpq_numref(r), xnum_yden, xden_ynum);

	mpz_clear(xnum_yden);
	mpz_clear(xden_ynum);
	return rat(tag, r);
}

Expr* comp(Tag tag, Expr** v, size_t n) {
	assert(n);
	auto x = v[0];
	// TODO: other simplifications e.g. x+0, x*1
	switch (tag) {
	case Tag::add:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_add, mpq_add);
		break;
	}
	case Tag::ceil:
		// Could do type-based simplification in cases like this, e.g. if x is an integer, then ceil(x) = x. But the value of that
		// is questionable: Is anyone going to write ceil(x) where x is an integer, in the first place?
		if (constant(x)) return op1(x, mpz_cdiv_q);
		break;
	case Tag::div:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, 0, mpq_div);
		break;
	}
	case Tag::divEuclid:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_ediv_q);
		break;
	}
	case Tag::divFloor:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_fdiv_q);
		break;
	}
	case Tag::divTrunc:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_tdiv_q);
		break;
	}
	case Tag::eq:
	{
		auto y = v[1];
		if (x == y) return bools + 1;
		if (constant(x) && constant(y)) return bools;
		break;
	}
	case Tag::floor:
		if (constant(x)) return op1(x, mpz_fdiv_q);
		break;
	case Tag::isInt:
		if (constant(x)) return bools + (mpz_cmp_ui(mpq_denref(((Rat*)x)->val), 1) == 0);
		break;
	case Tag::isRat:
		if (constant(x)) return bools + 1;
	case Tag::lt:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) {
			auto tag = x->tag;
			assert(tag == y->tag);
			if (tag == Tag::integer) return bools + (mpz_cmp(((Int*)x)->val, ((Int*)y)->val) < 0);
			return bools + (mpq_cmp(((Rat*)x)->val, ((Rat*)y)->val) < 0);
		}
		break;
	}
	case Tag::minus:
		if (constant(x)) {
			auto tag = x->tag;
			if (tag == Tag::integer) {
				mpz_t r;
				mpz_init(r);
				mpz_neg(r, ((Int*)x)->val);
				return integer(r);
			}
			mpq_t r;
			mpq_init(r);
			mpq_neg(r, ((Rat*)x)->val);
			return rat(tag, r);
		}
		break;
	case Tag::mul:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_mul, mpq_mul);
		break;
	}
	case Tag::remEuclid:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_ediv_r);
		break;
	}
	case Tag::remFloor:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_fdiv_r);
		break;
	}
	case Tag::remTrunc:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_tdiv_r);
		break;
	}
	case Tag::round:
		if (constant(x)) return op1(x, mpz_round);
		break;
	case Tag::sub:
	{
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_sub, mpq_sub);
		break;
	}
	case Tag::toInt:
		if (constant(x)) {
			auto x1 = (Rat*)x;
			mpz_t r;
			mpz_init(r);
			// Different languages have different conventions on the default rounding mode for converting fractions to integers.
			// TPTP defines it as floor, so that is used here. To use a different rounding mode, explicity round the rational number
			// first, and then convert to integer.
			mpz_fdiv_q(r, mpq_numref(x1->val), mpq_denref(x1->val));
			return integer(r);
		}
		break;
	case Tag::toRat:
		if (constant(x)) return toRat(x, Tag::rat);
		break;
	case Tag::toReal:
		if (constant(x)) return toRat(x, Tag::real);
		break;
	case Tag::trunc:
		if (constant(x)) return op1(x, mpz_tdiv_q);
		break;
	}
	return comps.intern(tag, v, n);
}

Expr* comp(Tag tag, Expr* a) {
	return comp(tag, &a, 1);
}

Expr* comp(Tag tag, Expr* a, Expr* b) {
	static Expr* v[2];
	v[0] = a;
	v[1] = b;
	return comp(tag, v, 2);
}

Expr* comp(Tag tag, const Vec<Expr*>& v) {
	return comps.intern(tag, v.data, v.n);
}

Expr* comp(Tag tag, vector<Expr*>& v) {
	return comps.intern(tag, v.data(), v.size());
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
	case Tag::isInt:
	case Tag::isRat:
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
	case Tag::toInt:
		return &tinteger;
	case Tag::rat:
	case Tag::toRat:
		return &trat;
	case Tag::real:
	case Tag::toReal:
		return &treal;
	case Tag::var:
		return ((Var*)a)->ty;
	}
	unreachable;
}
