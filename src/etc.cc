#include "main.h"

static void freeVars(set<term> boundv, term a, set<term>& freev) {
	switch (tag(a)) {
	case tag::All:
	case tag::Exists:
		for (size_t i = 2; i != a.size(); ++i) boundv.add(a[i]);
		freeVars(boundv, a[1], freev);
		return;
	case tag::Var:
		if (boundv.count(a)) return;
		freev.add(a);
		return;
	}
	for (size_t i = 1; i != a.size(); ++i) freeVars(boundv, a[i], freev);
}

// SORT
equation eqn(term a) {
	if (tag(a) == tag::Eq) return make_pair(a[1], a[2]);
	return make_pair(a, tag::True);
}

static void flatten(tag t, term a, vec<term>& r) {
	if (tag(a) == t) {
		for (size_t i = 1; i != a.size(); ++i) flatten(t, a[i], r);
		return;
	}
	r.push_back(a);
}

vec<term> flatten(tag t, term a) {
	vec<term> r;
	flatten(t, a, r);
	return r;
}

set<term> freeVars(term a) {
	set<term> r;
	freeVars(set<term>(), a, r);
	return r;
}

term imp(term a, term b) {
	return term(tag::Or, term(tag::Not, a), b);
}

bool occurs(term a, term b) {
	if (a == b) return 1;
	for (size_t i = 1; i != b.size(); ++i)
		if (occurs(a, b[i])) return 1;
	return 0;
}

term quantify(term a) {
	auto vars = freeVars(a);
	if (vars.empty()) return a;
	vec<term> v(1, term(tag::All));
	v.push_back(a);
	for (auto x: vars) v.push_back(x);
	return term(v);
}

// Are clauses sets of literals, or bags? It would seem logical to represent them as sets, and some algorithms prefer it that way,
// but unfortunately there are important algorithms that will break unless they are represented as bags, such as the superposition
// calculus:
// https://stackoverflow.com/questions/29164610/why-are-clauses-multisets
// So we represent them as bags (or lists, ignoring the order) and let the algorithms that prefer sets, discard duplicate literals.
clause uniq(const clause& c) {
	vec<term> neg;
	for (auto& a: c.first)
		if (find(neg.begin(), neg.end(), a) == neg.end()) neg.push_back(a);

	vec<term> pos;
	for (auto& a: c.second)
		if (find(pos.begin(), pos.end(), a) == pos.end()) pos.push_back(a);

	return make_pair(neg, pos);
}

set<clause> uniq(const set<clause>& cs) {
	set<clause> r;
	for (auto& c: cs) r.add(uniq(c));
	return r;
}
///
