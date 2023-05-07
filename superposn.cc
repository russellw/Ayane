#include "main.h"

namespace {
// First-order logic usually takes the view that equality is a special case, but superposition calculus takes the view that equality
// is the general case. Non-equality predicates are considered to be equations 'p=true'; this is a special exemption from the usual
// rule that equality is not allowed on Boolean terms.
bool equatable(Ex* a, Ex* b) {
	if (type(a) != type(b)) return 0;
	if (type(a) == &tbool) return a == bools + 1 || b == bools + 1;
	return 1;
}

// TODO: -> eqn.ex()?
Ex* equate(Ex* a, Ex* b) {
	assert(equatable(a, b));
	if (a == bools + 1) return b;
	if (b == bools + 1) return a;
	assert(type(a) != &tbool);
	assert(type(b) != &tbool);
	return ex(Eq, a, b);
}

// Equality tends to generate a large number of clauses. Superposition calculus is designed to moderate the profusion of clauses
// using an ordering on terms, that tries to apply equations in one direction only; the difficulty, of course, is doing this without
// breaking completeness.
// TODO: fixup to take into account 0 vs 1 arg start
// TODO: compare with KBO

// The greater-than test is supposed to be called on complete terms, which can include constant symbols (zero arity), or calls of
// function symbols (positive arity) with arguments. Make sure it's not being called on an isolated function symbol.
void check(Ex* a) {
	// TODO: is this still valid?
	assert(a->n);
}

size_t fn(Ex* a) {
	if (a->tag == Call) return (size_t)at(a, 0);
	return a->tag;
}

bool gt(Ex* a, Ex* b);

bool ge(Ex* a, Ex* b) {
	return a == b || gt(a, b);
}

// TODO: some analysis on the operators used in the clauses, to figure out what order is
// Likely to be best. For now, just use an arbitrary order.

// Check whether one term is unambiguously greater than another. This is much more delicate than comparison for e.g. sorting, where
// arbitrary choices can be made; to avoid breaking completeness of the calculus, the criteria are much stricter, and when in doubt,
// we return false.
bool gt(Ex* a, Ex* b) {
	check(a);
	check(b);

	// Fast equality test
	if (a == b) return 0;

	// Variables are unordered unless contained in other term
	// TODO: check how that relates to variable identity between clauses
	if (a->tag == Var) return 0;
	if (b->tag == Var) return occurs(b, a);

	// Sufficient condition: Exists ai >= b
	// TODO: check generated code
	for (size_t i = 0; i < a->n; ++i)
		if (ge(at(a, i), b)) return 1;

	// Necessary condition: a > all bi
	for (size_t i = 0; i < b->n; ++i)
		if (!gt(a, at(b, i))) return 0;

	// Different functions. Comparison has the required property that true is considered smaller than any other term (except false,
	// which does not occur during superposition proof search).
	auto af = fn(a);
	auto bf = fn(b);
	if (af != bf) return af > bf;

	// Same functions should mean similar terms
	assert(a->tag == b->tag);
	assert(a->n == b->n);
	assert(at(a, 0) == at(b, 0));

	// Lexicographic extension
	for (size_t i = 0; i < a->n; ++i) {
		if (gt(at(a, i), at(b, i))) return 1;
		if (at(a, i) != at(b, i)) return 0;
	}

	// Having found no differences, the terms must be equal, but we already checked for that first thing, so something is wrong
	unreachable;
}

// Inputs
Clause* c;
size_t ci;
Ex* c0;
Ex* c1;

Clause* d;
size_t di;
Ex* d0;
Ex* d1;

/*
equality resolution
	c | c0 != c1
->
	c/m
where
	m = unify(c0, c1)
*/

// Make new clause
void resolve1() {
	assert(!neg.n);
	assert(!pos.n);

	for (auto i: c->neg())
		if (i != ci) neg.add(replace(at(c, i), 0));

	for (auto i: c->pos()) pos.add(replace(at(c, i), 0));

	clause();
}

// For each negative equation
void resolve() {
	for (auto i: c->neg()) {
		auto e = eqn(at(c, i));
		if (unify(e.first, 0, e.second, 0)) {
			ci = i;
			resolve1();
		}
	}
}

/*
equality factoring
	c | c0 = c1 | d0 = d1
->
	(c | c0 = c1 | c1 != d1)/m
where
	m = unify(c0, d0)
*/

// Make new clause
void factorc() {
	assert(!neg.n);
	assert(!pos.n);

	// If these two terms are not equatable (for which the types must match, and predicates can only be equated with true),
	// replacing variables with expressions would not make them become so
	if (!equatable(c1, d1)) return;

	// The first expression of the second equation is called 'd0' because in the superposition rule, below, it will be an equation
	// in the second clause. Here, however, it is still a second equation in the first clause, so unification is done with the same
	// variable subscript on both sides.
	if (!unify(c0, 0, d0, 0)) return;

	for (auto i: c->neg()) neg.add(replace(at(c, i), 0));
	neg.add(equate(replace(c1, 0), replace(d1, 0)));

	for (auto i: c->pos())
		if (i != di) pos.add(replace(at(c, i), 0));

	clause();
}

// For each positive equation (both directions) again
void factor1() {
	for (auto i: c->pos()) {
		if (i == ci) continue;
		di = i;
		auto e = eqn(at(c, i));

		d0 = e.first;
		d1 = e.second;
		factorc();

		d0 = e.second;
		d1 = e.first;
		factorc();
	}
}

// For each positive equation (both directions)
void factor() {
	for (auto i: c->pos()) {
		auto e = eqn(at(c, i));
		ci = i;

		c0 = e.first;
		c1 = e.second;
		factor1();

		c0 = e.second;
		c1 = e.first;
		factor1();
	}
}

/*
superposition
	c | c0 = c1, d | d0(a) ?= d1
->
	(c | d | d0(c1) ?= d1)/m
where
	m = unify(c0, a)
	a is not a variable
*/

// The literature describes negative and positive superposition as separate inference rules; the only difference between them is
// whether they consider negative or positive equations in the second clause, so to avoid copy-pasting a significant chunk of
// nontrivial and almost identical code, we specify here a single inference rule

vec<size_t> posn;

Ex* splice(Ex* a, size_t i, Ex* b) {
	if (i == posn.size()) return b;

	assert(a->n);
	vec<Ex*> v(a->n);
	for (size_t j = 0; j < a->n; ++j) {
		v[j] = at(a, j);
		if (j == posn[i]) v[j] = splice(v[j], i + 1, b);
	}
	return ex(a->tag, v);
}

// Make new clause
void superposnc() {
	assert(!neg.n);
	assert(!pos.n);

	// To calculate d0(c1), we first perform the replacement of variables with substitute values, on the component terms, then
	// splice them together. This is necessary because the component terms are from different clauses, therefore have different
	// logical variable names. The composition would not be valid if we were replacing arbitrary terms, but is valid because we are
	// only replacing variables.
	auto d0c1 = splice(replace(d0, 1), 0, replace(c1, 0));
	auto d1_ = replace(d1, 1);
	if (!equatable(d0c1, d1_)) return;

	for (auto i: c->neg()) neg.add(replace(at(c, i), 0));
	for (auto i: d->neg())
		if (i != di) neg.add(replace(at(d, i), 1));

	for (auto i: c->pos())
		if (i != ci) pos.add(replace(at(c, i), 0));
	for (auto i: d->pos())
		if (i != di) pos.add(replace(at(d, i), 1));

	auto& v = di < d->nn ? neg : pos;
	v.add(equate(d0c1, d1_));

	clause();
}

// Descend into subterms
void descend(Ex* a) {
	// It is never necessary to paramodulate into variables
	if (a->tag == Var) return;

	// a is a subexpression of d, so its variables are subscripted 1
	if (unify(c0, 0, a, 1)) superposnc();

	// TODO: Call -> start at 1
	// c0 could unify with a, some subexpression of a, or both
	for (size_t i = 0; i < a->n; ++i) {
		posn.add(i);
		descend(at(a, i));
		--posn.n;
	}
}

// For each equation in d (both directions)
void superposn2() {
	// TODO: check this
	if (c0 == bools + 1) return;
	for (auto i: d) {
		auto e = eqn(at(d, i));
		di = i;

		assert(!posn.n);
		d0 = e.first;
		d1 = e.second;
		descend(d0);

		assert(!posn.n);
		d0 = e.second;
		d1 = e.first;
		descend(d0);
	}
}

// For each positive equation in c (both directions)
void superposn1() {
	for (auto i: c->pos()) {
		auto e = eqn(at(c, i));
		ci = i;

		c0 = e.first;
		c1 = e.second;
		superposn2();

		c0 = e.second;
		c1 = e.first;
		superposn2();
	}
}
} // namespace

// If a complete saturation proof procedure finds no more possible derivations, then the problem is satisfiable. In practice, this
// almost never happens for nontrivial problems, but serves as a good way to test the completeness of the prover on some trivial
// problems. However, if completeness was lost for any reason, then we will need to report failure instead.
int result = 1;

void superposn() {
	// The active set starts off empty
	vec<Clause*> active;

	// Saturation proof procedure tries to perform all possible derivations until it derives a contradiction
	for (;;) {
	loop:
		// If there are no more clauses in the queue, the problem is satisfiable, unless completeness was lost
		if (passive.empty()) return;
		incStat("superposn main loop");

		// Given clause
		auto g = passive.top();
		passive.pop();

		// Derived false
		if (!g->n) {
			result = 0;
			return;
		}

		// This is the Discount loop (in which only active clauses participate in subsumption checks); in tests, it performed
		// slightly better than the alternative Otter loop (in which passive clauses also participate)

		// Forward subsumption
		for (auto h: active)
			if (!h->dead && subsumes(h, g)) goto loop;

		// Backward subsumption
		for (auto h: active)
			if (!h->dead && subsumes(g, h)) h->dead = 1;

		// Add g to active clauses before inference, because we will sometimes need to combine g with itself
		active.add(g);

		// Infer
		c = g;
		resolve();
		factor();
		for (auto h: active) {
			if (h->dead) continue;

			c = h;
			d = g;
			superposn1();

			c = g;
			d = h;
			superposn1();
		}
	}
}
