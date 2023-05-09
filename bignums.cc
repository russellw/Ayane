#include "main.h"

// Numbers need to be interned, to preserve the property that equal terms have pointer equality, and in particular that they have
// the same hash codes

// TODO: divide this functionality between terms and simplify?
// TODO: write test problems for integer division
// Integers
bool eq(int tag, mpz_t a, size_t n, Ex* b) {
	return mpz_cmp(a, b->mpz) == 0;
}

size_t hash(mpz_t a) {
	return mpz_get_ui(a);
}

struct IntegerCmp {};

static Set<int, mpz_t, Ex, IntegerCmp> integers;

// Rationals
bool eq(int tag, mpq_t a, size_t n, Ex* b) {
	return mpq_equal(a, b->mpq);
}

size_t hash(mpq_t a) {
	return hashCombine(mpz_get_ui(mpq_numref(a)), mpz_get_ui(mpq_denref(a)));
}

struct RationalCmp {};

static Set<int, mpq_t, Ex, RationalCmp> rationals;

Ex* real(mpq_t q) {
	return ex(ToReal, intern(q));
}

// Arithmetic
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
