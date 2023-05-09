#include "main.h"

// Arithmetic
Expr* minus(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_neg(r, a->mpz);
		return intern(r);
	}
	case Tag::rational:
	{
		mpq_t r;
		mpq_init(r);
		mpq_neg(r, a->mpq);
		return intern(r);
	}
	}
	unreachable;
}

Expr* remTrunc(Expr* a, Expr* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Tag::integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_tdiv_r(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Tag::rational:
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

Expr* remFloor(Expr* a, Expr* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Tag::integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_fdiv_r(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Tag::rational:
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

Expr* remEuclid(Expr* a, Expr* b) {
	assert(a->tag == b->tag);
	switch (a->tag) {
	case Tag::integer:
	{
		mpz_t r;
		mpz_init(r);
		mpz_ediv_r(r, a->mpz, b->mpz);
		return intern(r);
	}
	case Tag::rational:
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

Expr* ceil(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
		return a;
	case Tag::rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_cdiv_q(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Expr* floor(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
		return a;
	case Tag::rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_fdiv_q(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Expr* trunc(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
		return a;
	case Tag::rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_tdiv_q(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

Expr* round(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
		return a;
	case Tag::rational:
	{
		mpq_t r;
		mpq_init(r);
		mpz_round(mpq_numref(r), mpq_numref(a->mpq), mpq_denref(a->mpq));
		return intern(r);
	}
	}
	unreachable;
}

bool isInteger(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
		return 1;
	case Tag::rational:
		return !mpz_cmp_ui(mpq_denref(a->mpq), 1);
	}
	unreachable;
}

Expr* toInteger(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
		return a;
	case Tag::rational:
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

Expr* toRational(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
	{
		mpq_t r;
		mpq_init(r);
		mpz_set(mpq_numref(r), a->mpz);
		return intern(r);
	}
	case Tag::rational:
		return a;
	}
	unreachable;
}

Expr* toReal(Expr* a) {
	switch (a->tag) {
	case Tag::integer:
	{
		mpq_t r;
		mpq_init(r);
		mpz_set(mpq_numref(r), a->mpz);
		return real(r);
	}
	case Tag::rational:
		// TODO: implement this in terms of changing the tag
		return expr(Tag::toReal, a);
	}
	unreachable;
}
