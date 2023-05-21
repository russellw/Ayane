#include "main.h"

namespace {
Vec<pair<Expr*, Expr*>> m;

bool match(Expr* a, Expr* b) {
	// Equals
	if (a == b) return 1;

	// Type mismatch
	if (type(a) != type(b)) return 0;

	// Variable
	if (a->tag == Tag::var) {
		Expr* y;
		if (get(a, y, m)) return y == b;
		m.add(make_pair(a, b));
		return 1;
	}

	// Different operators
	if (a->tag != b->tag) return 0;

	// If nonvariable leaves could match, they would already have tested equal
	if (!a->n) return 0;

	// Recur
	if (a->n != b->n) return 0;
	for (size_t i = 0; i < a->n; ++i)
		if (!match(at(a, i), at(b, i))) return 0;
	return 1;
}

bool match(Eqn a, Eqn b) {
	auto o = m.n;

	if (match(a.first, b.first) && match(a.second, b.second)) return 1;
	m.n = o;

	if (match(a.first, b.second) && match(a.second, b.first)) return 1;
	m.n = o;

	return 0;
}

Clause* c;
Range cs;

Clause* d;
Range ds;

// Multiset avoids breaking completeness when factoring is used
bool used[100];

bool subsume(size_t ci) {
	if (ci == cs.second) return 1;
	auto a = at(c, ci++);
	for (auto di: ds) {
		if (used[di]) continue;

		auto b = at(d, di);
		if (!match(a, b)) continue;

		auto o = m.n;
		used[di] = 1;
		if (subsume(ci)) return 1;
		used[di] = 0;
		m.n = o;
	}
	return 0;
}
} // namespace

bool subsumes(Clause* c0, Clause* d0) {
	// It is impossible for a longer clause to subsume a shorter one
	if (c0->nn > d0->nn) return 0;
	if (c0->np() > d0->np()) return 0;

	// It's okay to use a fixed-size array here, because beyond a certain length, subsumption checking is unlikely to be useful.
	// Indeed, beyond a certain length, generating clauses is unlikely to be useful.
	if (sizeof used < d0->n) return 0;

	c = c0;
	d = d0;
	m.n = 0;
	memset(used, 0, d->n);

	cs = c->neg();
	ds = d->neg();
	if (!subsume(cs.first)) return 0;

	cs = c->pos();
	ds = d->pos();
	if (!subsume(cs.first)) return 0;

	return 1;
}
