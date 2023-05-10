#include "main.h"

char buf[5000];

// SORT
Eqn::Eqn(Expr* a) {
	// TODO: move to same module as the definition of the structure
	if (a->tag == Tag::eq) {
		first = at(a, 0);
		second = at(a, 1);
		return;
	}
	first = a;
	second = bools + 1;
}

void flatten(Tag tag, Expr* a, vector<Expr*>& r) {
	if (a->tag == tag) {
		for (size_t i = 0; i < a->n; ++i) flatten(tag, at(a, i), r);
		return;
	}
	r.push_back(a);
}

void freeVars(Expr* a, Vec<Expr*> boundv, Vec<Expr*>& freev) {
	// TODO: boundv static?
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
	{
		auto o = boundv.n;
		// TODO: batch add
		for (size_t i = 1; i < a->n; ++i) boundv.add(at(a, i));
		freeVars(at(a, 0), boundv, freev);
		boundv.n = o;
		return;
	}
	case Tag::var:
		if (!boundv.has(a) && !freev.has(a)) freev.add(a);
		return;
	}
	for (size_t i = 0; i < a->n; ++i) freeVars(at(a, i), boundv, freev);
}

Expr* imp(Expr* a, Expr* b) {
	return comp(Tag::or1, comp(Tag::not1, a), b);
}

void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d) {
	mpz_t r;
	mpz_init(r);
	mpz_ediv_r(r, n, d);
	mpz_sub(q, n, r);
	mpz_clear(r);
	mpz_tdiv_q(q, q, d);
}

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

// Calculate q = n/d, assuming common factors have already been canceled out, and applying bankers rounding
void mpz_round(mpz_t q, const mpz_t n, const mpz_t d) {
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

bool occurs(Expr* a, Expr* b) {
	if (a == b) return 1;
	for (size_t i = 0; i < b->n; ++i)
		if (occurs(a, at(b, i))) return 1;
	return 0;
}

Expr* quantify(Expr* a) {
	Vec<Expr*> vars;
	freeVars(a, Vec<Expr*>(), vars);
	if (vars.empty()) return a;
	Vec<Expr*> v(1, a);
	// TODO: add all at once
	for (auto x: vars) v.add(x);
	return comp(Tag::all, v);
}
///
