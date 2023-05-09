#include "main.h"

namespace {
Vec<pair<Expr*, Expr*>> m;

bool match(Expr* a, Expr* b) {
	// Equals
	if (eq(a, 0, b, 1)) return 1;

	// Type mismatch
	if (type(a) != type(b)) return 0;

	// Variable
	// TODO: check variables more efficiently
	if (a->tag == Tag::var) {
		// TODO: check generated code
		if (get(a, a, m)) return a == b;
		m.add(make_pair(a, b));
		return 1;
	}

	// Mismatched tags
	if (a->tag != b->tag) return 0;

	// If nonvariable atoms could match, they would already have tested equal
	// TODO: check generated code
	auto n = a->n;
	if (!n) return 0;

	// Recur
	if (b->n != n) return 0;
	for (size_t i = 0; i < n; ++i)
		if (!match(at(a, i), at(b, i))) return 0;
	return 1;
}

bool match(Eqn a, Eqn b) {
	auto o = m.size();

	if (match(a.first, b.first) && match(a.second, b.second)) return 1;
	m.resize(o);

	if (match(a.first, b.second) && match(a.second, b.first)) return 1;
	m.resize(o);

	return 0;
}

Clause* c;
Range cs;

Clause* d;
Range ds;

// Multiset avoids breaking completeness when factoring is used
Vec<bool> used;

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
	memset(used.data, 0, used.size());

	cs = c->neg();
	ds = d->neg();
	if (!subsume(cs.first)) return 0;

	cs = c->pos();
	ds = d->pos();
	if (!subsume(cs.first)) return 0;

	return 1;
}
