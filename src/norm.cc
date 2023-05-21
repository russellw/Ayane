#include "main.h"

namespace {
bool constant(Expr* a) {
	switch (a->tag) {
	case Tag::distinctObj:
	case Tag::integer:
	case Tag::rat:
	case Tag::real:
		return 1;
	default:
		return 0;
	}
}

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
} // namespace

Expr* norm(Expr* a) {
	if (!a->n) {
		if (a->tag == Tag::var) {}
		return a;
	}
	Vec<Expr*> v(a->n);
	for (size_t i = 0; i < a->n; ++i) v[i] = norm(at(a, i));
	// TODO: other simplifications e.g. x+0, x*1
	switch (a->tag) {
	case Tag::add:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_add, mpq_add);
		break;
	}
	case Tag::ceil:
	{
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_cdiv_q);
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
	case Tag::eq:
	{
		auto x = v[0];
		auto y = v[1];
		if (x == y) return bools + 1;
		if (constant(x) && constant(y)) return bools;
		break;
	}
	case Tag::floor:
	{
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_fdiv_q);
		break;
	}
	case Tag::isInt:
	{
		auto x = v[0];
		if (type(x) == &tinteger) return bools + 1;
		if (constant(x)) return bools + (mpz_cmp_ui(mpq_denref(((Rat*)x)->v), 1) == 0);
		break;
	}
	case Tag::isRat:
	{
		auto x = v[0];
		if (constant(x)) return bools + 1;
		break;
	}
	case Tag::lt:
	{
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
	case Tag::round:
	{
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_round);
		break;
	}
	case Tag::sub:
	{
		auto x = v[0];
		auto y = v[1];
		if (constant(x) && constant(y)) return op2(x, y, mpz_sub, mpq_sub);
		break;
	}
	case Tag::toInt:
	{
		auto x = v[0];
		if (type(x) == &tinteger) return x;
		if (constant(x)) {
			auto x1 = (Rat*)x;
			mpz_t r;
			mpz_init(r);
			// TODO: should fmt put blank line before comment?
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
		auto x = v[0];
		if (constant(x)) return toRat(x, Tag::rat);
		break;
	}
	case Tag::toReal:
	{
		auto x = v[0];
		if (constant(x)) return toRat(x, Tag::real);
		break;
	}
	case Tag::trunc:
	{
		auto x = v[0];
		if (constant(x)) return op1(x, mpz_tdiv_q);
		break;
	}
	default:
		break;
	}
	return compc(a->tag, v);
}
