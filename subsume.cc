#include "all.h"

namespace {
vec<pair<Ex*, Ex*>> m;

bool match(ex a, ex b) {
	// Equals
	if (eq(a, 0, b, 1)) return 1;

	// Type mismatch
	if (type(a) != type(b)) return 0;

	// Variable
	// TODO: check variables more efficiently
	if (a->tag == Var) {
		auto& ma = m.gadd(a);

		// Existing mapping. First-order variables cannot be Boolean, which has the useful corollary that the default value of a
		// term (false) is distinguishable from any term to which a variable could be validly mapped.
		if (ma.raw) return ma == b;

		// New mapping
		ma = b;
		return 1;
	}

	// Mismatched tags
	if (a->tag != b->tag) return 0;

	// If nonvariable atoms could match, they would already have tested equal
	auto n = a.size();
	if (!n) return 0;

	// Recur
	if (b.size() != n) return 0;
	for (size_t i = 0; i < n; ++i)
		if (!match(m, at(a, i), b[i])) return 0;
	return 1;
}

bool match(eqn a, eqn b) {
	auto o = m.size();

	if (match(a.first, b.first) && match(a.second, b.second)) return 1;
	m.resize(o);

	if (match(a.first, b.second) && match(a.second, b.first)) return 1;
	m.resize(o);

	return 0;
}

Clause* c;
range cs;

Clause* d;
range ds;

// Multiset avoids breaking completeness when factoring is used
vec<bool> used;

bool subsume(int ci) {
	if (ci == cs.second) return 1;
	auto a = at(c, ci++);
	for (auto di: ds) {
		if (used[di]) continue;

		auto b = at(d, di);
		if (!match(a, b)) continue;

		auto o = m.size();
		used[di] = 1;
		if (subsume(ci)) return 1;
		used[di] = 0;
		m.resize(o);
	}
	return 0;
}
} // namespace

bool subsumes(Clause* c0, Clause* d0) {
	if (c0->n > d0->n) return 0;

	c = c0;
	d = d0;
	m.clear();

	used.resize(d->n);
	memset(used.data(), 0, used.size());

	cs = c->neg();
	ds = d->neg();
	if (!subsume(cs.first)) return 0;

	cs = c->pos();
	ds = d->pos();
	if (!subsume(cs.first)) return 0;

	return 1;
}
