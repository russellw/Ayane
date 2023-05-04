#include "all.h"

namespace {
vec<pair<Ex*, Ex*>> unified;

bool subunify_var(Ex* a, Ex* b) {
	assert(a->tag == t_var);
	assert(a->ty() == b->ty());

	for (auto p: unified)
		if (p.first == a) return eq(p.second, b);

	unified.push_back(make_pair(a, b));
	return true;
}

bool subunify(Ex* a, Ex* b) {
	assert(a->ty() == b->ty());

	// Same Ex
	if (a == b) return true;

	// Variables
	if (a->tag == t_var) return subunify_var(a, b);
	if (a->tag != b->tag) return false;

	// Atoms
	if (!a->n) return eq(a, b);

	// Compound terms
	if (a->n != b->n) return false;
	for (auto i: a)
		if (!subunify(at(a, i), at(b, i))) return false;
	return true;
}

bool subunify(eqn a, eqn b) {
	if (a.first->ty() != b.first->ty()) return false;

	auto n = unified.size();

	if (subunify(a.first, b.first) && subunify(a.second, b.second)) return true;
	unified.resize(n);

	if (subunify(a.first, b.second) && subunify(a.second, b.first)) return true;
	unified.resize(n);

	return false;
}

Clause* c;
range cs;

Clause* d;
range ds;

// Multiset avoids breaking completeness when factoring is used
vec<bool> used;

bool subsume(int ci) {
	if (ci == cs.second) return true;
	auto a = at(c, ci++);
	for (auto di: ds) {
		if (used[di]) continue;

		auto b = at(d, di);
		if (!subunify(a, b)) continue;

		auto n = unified.size();
		used[di] = true;
		if (subsume(ci)) return true;
		used[di] = false;
		unified.resize(n);
	}
	return false;
}
} // namespace

bool subsumes(Clause* c0, Clause* d0) {
	if (c0->n > d0->n) return false;

	c = c0;
	d = d0;
	unified.clear();

	used.resize(d->n);
	memset(used.data(), 0, used.size());

	cs = c->neg();
	ds = d->neg();
	if (!subsume(cs.first)) return false;

	cs = c->pos();
	ds = d->pos();
	if (!subsume(cs.first)) return false;

	return true;
}
