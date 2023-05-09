#include "main.h"

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
