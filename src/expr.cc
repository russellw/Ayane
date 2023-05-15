#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// Numbers need to be interned, to preserve the property that equal terms have pointer equality

// Integers
struct IntCmp {
	static bool eq(Tag tag, mpz_t v, size_t n, Int* b) {
		return mpz_cmp(v, b->v) == 0;
	}
	static bool eq(Int* a, Int* b) {
		return mpz_cmp(a->v, b->v) == 0;
	}

	static size_t hash(Tag tag, mpz_t v, size_t n) {
		return mpz_get_ui(v);
	}
	static size_t hash(Int* a) {
		return hash(Tag::integer, ((Int*)a)->v, 0);
	}

	static Int* make(Tag tag, mpz_t v, size_t n) {
		return new Int(v);
	}
};

void clear(mpz_t v) {
	mpz_clear(v);
}

static Set<Tag, mpz_t, Int, IntCmp> integers;

Int* integer(mpz_t v) {
	return integers.intern(Tag::integer, v, 0);
}

// Rationals
struct RatCmp {
	// TODO: can we now remove the Cmp class?
	static bool eq(Tag tag, mpq_t v, size_t n, Rat* b) {
		return tag == b->tag && mpq_equal(v, b->v);
	}
	static bool eq(Rat* a, Rat* b) {
		return a->tag == b->tag && mpq_equal(a->v, b->v);
	}

	static size_t hash(Tag tag, mpq_t v, size_t n) {
		return hashCombine(mpz_get_ui(mpq_numref(v)), mpz_get_ui(mpq_denref(v)));
	}
	static size_t hash(Rat* a) {
		return hash(a->tag, ((Rat*)a)->v, 0);
	}

	static Rat* make(Tag tag, mpq_t v, size_t n) {
		return new Rat(tag, v);
	}
};

void clear(mpq_t v) {
	mpq_clear(v);
}

static Set<Tag, mpq_t, Rat, RatCmp> rats;

Rat* rat(Tag tag, mpq_t v) {
	return rats.intern(tag, v, 0);
}

// Composite expressions
Expr* comp(Tag tag, Expr** v, size_t n) {
	auto a = (Comp*)malloc(sizeof(Comp) + n * sizeof *v);
	a->tag = tag;
	a->n = n;
	memcpy(a->v, v, n * sizeof *v);
	return a;
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

Expr* comp(Tag tag, Vec<Expr*>& v) {
	return comp(tag, v.data, v.n);
}

Expr* comp(Tag tag, vector<Expr*>& v) {
	// TODO: sizeof void* consistently
	return comp(tag, v.data(), v.size());
}

struct CompCmp {
	static bool eq(Tag tag, Expr** a, size_t n, Comp* b) {
		assert(a);
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

	static Comp* make(Tag tag, Expr** v, size_t n) {
		auto p = malloc(sizeof(Comp) + n * sizeof *v);
		auto a = new (p) Comp(tag, n);
		memcpy(a->v, v, n * sizeof *v);
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
	if (type(x) == &tinteger) return x;

	auto x1 = (Rat*)x;

	mpq_t r;
	mpq_init(r);
	f(mpq_numref(r), mpq_numref(x1->v), mpq_denref(x1->v));
	return rat(x->tag, r);
}

Rat* toRat(Expr* x, Tag tag) {
	mpq_t r;
	mpq_init(r);
	if (x->tag == Tag::integer) mpz_set(mpq_numref(r), ((Int*)x)->v);
	else
		mpq_set(r, ((Rat*)x)->v);
	return rat(tag, r);
}

Expr* op2(Expr* x, Expr* y, void (*fz)(mpz_t, const mpz_t, const mpz_t), void (*fq)(mpq_t, const mpq_t, const mpq_t)) {
	auto tag = x->tag;
	assert(tag == y->tag);

	if (tag == Tag::integer) {
		mpz_t r;
		mpz_init(r);
		fz(r, ((Int*)x)->v, ((Int*)y)->v);
		return integer(r);
	}

	mpq_t r;
	mpq_init(r);
	fq(r, ((Rat*)x)->v, ((Rat*)y)->v);
	return rat(tag, r);
}

Expr* div2(Expr* x, Expr* y, void (*f)(mpz_t, const mpz_t, const mpz_t)) {
	auto tag = x->tag;
	assert(tag == y->tag);

	if (tag == Tag::integer) {
		mpz_t r;
		mpz_init(r);
		f(r, ((Int*)x)->v, ((Int*)y)->v);
		return integer(r);
	}

	auto x1 = (Rat*)x;
	auto y1 = (Rat*)y;

	mpz_t xnum_yden;
	mpz_init(xnum_yden);
	mpz_mul(xnum_yden, mpq_numref(x1->v), mpq_denref(y1->v));

	mpz_t xden_ynum;
	mpz_init(xden_ynum);
	mpz_mul(xden_ynum, mpq_denref(x1->v), mpq_numref(y1->v));

	mpq_t r;
	mpq_init(r);
	f(mpq_numref(r), xnum_yden, xden_ynum);

	mpz_clear(xnum_yden);
	mpz_clear(xden_ynum);
	return rat(tag, r);
}

Expr* compc(Tag tag, Expr** v, size_t n) {
	// TODO: other simplifications e.g. x+0, x*1
	switch (tag) {
	case Tag::add:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_add, mpq_add);
		break;
	}
	case Tag::ceil:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_cdiv_q);
		break;
	}
	case Tag::div:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, 0, mpq_div);
		break;
	}
	case Tag::divEuclid:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_ediv_q);
		break;
	}
	case Tag::divFloor:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_fdiv_q);
		break;
	}
	case Tag::divTrunc:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_tdiv_q);
		break;
	}
	case Tag::eq:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (x == y) return bools + 1;
		if (constant(x) && constant(y)) return bools;
		break;
	}
	case Tag::floor:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_fdiv_q);
		break;
	}
	case Tag::isInt:
	{
		assert(n == 1);
		auto x = v[0];
		if (type(x) == &tinteger) return bools + 1;
		if (constant(x)) return bools + (mpz_cmp_ui(mpq_denref(((Rat*)x)->v), 1) == 0);
		break;
	}
	case Tag::isRat:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return bools + 1;
		break;
	}
	case Tag::lt:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) {
			auto tag = x->tag;
			assert(tag == y->tag);
			if (tag == Tag::integer) return bools + (mpz_cmp(((Int*)x)->v, ((Int*)y)->v) < 0);
			return bools + (mpq_cmp(((Rat*)x)->v, ((Rat*)y)->v) < 0);
		}
		break;
	}
	case Tag::minus:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) {
			auto tag = x->tag;
			if (tag == Tag::integer) {
				mpz_t r;
				mpz_init(r);
				mpz_neg(r, ((Int*)x)->v);
				return integer(r);
			}
			mpq_t r;
			mpq_init(r);
			mpq_neg(r, ((Rat*)x)->v);
			return rat(tag, r);
		}
		break;
	}
	case Tag::mul:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_mul, mpq_mul);
		break;
	}
	case Tag::remEuclid:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_ediv_r);
		break;
	}
	case Tag::remFloor:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_fdiv_r);
		break;
	}
	case Tag::remTrunc:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return div2(x, y, mpz_tdiv_r);
		break;
	}
	case Tag::round:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_round);
		break;
	}
	case Tag::sub:
	{
		assert(n == 2);
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_sub, mpq_sub);
		break;
	}
	case Tag::toInt:
	{
		assert(n == 1);
		auto x = v[0];
		if (type(x) == &tinteger) return x;
		if (constant(x)) {
			auto x1 = (Rat*)x;
			mpz_t r;
			mpz_init(r);
			// Different languages have different conventions on the default rounding mode for converting fractions to integers.
			// TPTP defines it as floor, so that is used here. To use a different rounding mode, explicity round the rational number
			// first, and then convert to integer.
			mpz_fdiv_q(r, mpq_numref(x1->v), mpq_denref(x1->v));
			return integer(r);
		}
		break;
	}
	case Tag::toRat:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return toRat(x, Tag::rat);
		break;
	}
	case Tag::toReal:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return toRat(x, Tag::real);
		break;
	}
	case Tag::trunc:
	{
		assert(n == 1);
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_tdiv_q);
		break;
	}
	}
	return comps.intern(tag, v, n);
}

Expr* compc(Tag tag, Vec<Expr*>& v) {
	return compc(tag, v.data, v.n);
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

#ifdef DBG
void print(Tag tag) {
	static const char* tagNames[] = {
#define _(a) #a,
#include "tags.h"
	};
	print(tagNames[(int)tag]);
}

void print(Expr* a) {
	switch (a->tag) {
	case Tag::call:
		print(at(a, 0));
		putchar('(');
		for (size_t i = 1; i < a->n; ++i) {
			if (i > 1) print(", ");
			print(at(a, i));
		}
		putchar(')');
		return;
	case Tag::fn:
	{
		auto s = ((Fn*)a)->s;
		if (s) print(s);
		else
			printf("_%p", a);
		return;
	}
	case Tag::integer:
		mpz_out_str(stdout, 10, ((Int*)a)->v);
		return;
	case Tag::rat:
	case Tag::real:
		mpq_out_str(stdout, 10, ((Rat*)a)->v);
		return;
	case Tag::var:
		printf("%p", a);
		return;
	}
	print(a->tag);
	if (!a->n) return;
	putchar('(');
	for (size_t i = 0; i < a->n; ++i) {
		if (i) print(", ");
		print(at(a, i));
	}
	putchar(')');
}
#endif
