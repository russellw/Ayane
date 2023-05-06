#include "all.h"

// Numbers need to be interned, to preserve the property that equal terms have pointer equality, and in particular that they have
// the same hash codes

// TODO: divide this functionality between terms and simplify?
// TODO: write test problems for integer division
// Integers
size_t hash(mpz_t a) {
	return mpz_get_ui(a);
}

bool eq(Ex* a, mpz_t b) {
	return !mpz_cmp(a->mpz, b);
}

struct IntegerCmp {};

static set<mpz_t, Ex, IntegerCmp> integers;

// Rationals
size_t hash(mpq_t a) {
	return hashCombine(mpz_get_ui(mpq_numref(a)), mpz_get_ui(mpq_denref(a)));
}

bool eq(Ex* a, mpq_t b) {
	return mpq_equal(a->mpq, b);
}

struct RationalCmp {};

static set<mpq_t, Ex, RationalCmp> rationals;

Ex* real(mpq_t q) {
	return ex(ToReal, intern(q));
}

// Arithmetic
namespace {
void mpz_ediv_r(mpz_t r, const mpz_t n, const mpz_t d) {
	mpz_tdiv_r(r, n, d);
	if (mpz_sgn(r) < 0) {
		mpz_t dabs;
		mpz_init(dabs);
		mpz_abs(dabs, d);
		mpz_add(r, r, dabs);
		mpz_clear(dabs);
	}
}

void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d) {
	mpz_t r;
	mpz_init(r);
	mpz_ediv_r(r, n, d);
	mpz_sub(q, n, r);
	mpz_clear(r);
	mpz_tdiv_q(q, q, d);
}

// Calculate q = n/d, assuming common factors have already been canceled out, and applying bankers rounding
void mpz_round(mpz_t q, mpz_t n, mpz_t d) {
	// If we are dividing by 2, the result could be exactly halfway between two integers, so need special case to apply bankers
	// rounding
	if (!mpz_cmp_ui(d, 2)) {
		// Floored division by 2 (this corresponds to arithmetic shift right one bit)
		mpz_fdiv_q_2exp(q, n, 1);

		// If it was an even number before the division, the issue doesn't arise; we already have the exact answer
		if (!mpz_tstbit(n, 0)) return;

		// If it's an even number after the division, we are already on the nearest even integer, so we don't need to do anything
		// else
		if (!mpz_tstbit(q, 0)) return;

		// Need to adjust by one to land on an even integer, but which way? Floored division rounded down, so we need to go up.
		mpz_add_ui(q, q, 1);
		return;
	}

	// We are not dividing by 2, so cannot end up exactly halfway between two integers, and merely need to add half the denominator
	// to the numerator before dividing
	mpz_t d2;
	mpz_init(d2);
	mpz_fdiv_q_2exp(d2, d, 1);
	mpz_add(q, n, d2);
	mpz_clear(d2);
	mpz_fdiv_q(q, q, d);
}
} // namespace

Ex* minus(Ex* a) {
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_neg(r, a->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpq_neg(r, a->mpq);
		return intern(r);
	}
	}
	unreachable;
}

Ex* add(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_add(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpq_add(r, a->mpq, b->mpq);
		return intern(r);
	}
	}
	unreachable;
}

Ex* sub(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_sub(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpq_sub(r, a->mpq, b->mpq);
		return intern(r);
	}
	}
	unreachable;
}

Ex* mul(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_mul(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpq_mul(r, a->mpq, b->mpq);
		return intern(r);
	}
	}
	unreachable;
}

Ex* div(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);

		// TPTP does not define integer division with unspecified rounding mode, but most programming languages nowadays define it
		// as truncating
		// TODO: Does SMT-LIB use this?
		mpz_tdiv_q(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpq_div(r, a->mpq, b->mpq);
		return intern(r);
	}
	}
	unreachable;
}

Ex* divT(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_tdiv_q(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a->mpq), mpq_denref(b->mpq));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a->mpq), mpq_numref(b->mpq));

		mpq_t r;
		mpq_init(r);
		mpz_tdiv_q(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

Ex* divF(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_fdiv_q(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a->mpq), mpq_denref(b->mpq));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a->mpq), mpq_numref(b->mpq));

		mpq_t r;
		mpq_init(r);
		mpz_fdiv_q(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

Ex* divE(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_ediv_q(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a->mpq), mpq_denref(b->mpq));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a->mpq), mpq_numref(b->mpq));

		mpq_t r;
		mpq_init(r);
		mpz_ediv_q(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

Ex* remT(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_tdiv_r(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a->mpq), mpq_denref(b->mpq));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a->mpq), mpq_numref(b->mpq));

		mpq_t r;
		mpq_init(r);
		mpz_tdiv_r(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

Ex* remF(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_fdiv_r(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a->mpq), mpq_denref(b->mpq));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a->mpq), mpq_numref(b->mpq));

		mpq_t r;
		mpq_init(r);
		mpz_fdiv_r(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

Ex* remE(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_ediv_r(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Rational:
	{
		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a->mpq), mpq_denref(b->mpq));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a->mpq), mpq_numref(b->mpq));

		mpq_t r;
		mpq_init(r);
		mpz_ediv_r(mpq_numref(r), xnum_yden, xden_ynum);

		// TODO: free in reverse order?
		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

Ex* ceil(Ex* a) {
	switch (a->tag) {
	case Integer:
		return a;
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_cdiv_q(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Ex* floor(Ex* a) {
	switch (a->tag) {
	case Integer:
		return a;
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_fdiv_q(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Ex* trunc(Ex* a) {
	switch (a->tag) {
	case Integer:
		return a;
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_tdiv_q(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Ex* round(Ex* a) {
	switch (a->tag) {
	case Integer:
		return a;
	case Rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_round(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

bool isInteger(Ex* a) {
	switch (a->tag) {
	case Integer:
		return 1;
	case Rational:
		return !mpz_cmp_ui(mpq_denref(a->mpq), 1);
	}
	unreachable;
}

Ex* toInteger(Ex* a) {
	switch (a->tag) {
	case Integer:
		return a;
	case Rational:
	{
		mpz_t r;
		mpz_init(r);

		// Different languages have different conventions on the default rounding mode for converting fractions to integers. TPTP
		// defines it as floor, so that is used here. To use a different rounding mode, explicity round the rational number first,
		// and then convert to integer.
		mpz_fdiv_q(r, mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Ex* toRational(Ex* a) {
	switch (a->tag) {
	case Integer:
	{
		mpq_t r;
		mpq_init(r);
		mpz_set(mpq_numref(r), a->mpz);
		return intern(r);
	}
	case Rational:
		return a;
	}
	unreachable;
}

Ex* toReal(Ex* a) {
	switch (a->tag) {
	case Integer:
	{
		mpq_t r;
		mpq_init(r);
		mpz_set(mpq_numref(r), a->mpz);
		return real(r);
	}
	case Rational:
		return ex(ToReal, a);
	}
	unreachable;
}
