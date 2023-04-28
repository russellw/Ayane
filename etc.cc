#include "main.h"

const char* szsNames[] = {
#define _(x) #x,
#include "szs.h"
};

// SORT
Eqn eqn(Term* a) {
	if (a->tag == Eq) return make_pair(at(a, 1), at(a, 2));
	return make_pair(a, True);
}

static void flatten(int tag, Term* a, vec<Term*>& r) {
	if (a->tag == t) {
		for (size_t i = 1; i < a->n; ++i) flatten(t, at(a, i), r);
		return;
	}
	r.push_back(a);
}

vec<Term*> flatten(int tag, Term* a) {
	vec<Term*> r;
	flatten(t, a, r);
	return r;
}

set<Term*> freeVars(Term* a) {
	set<Term*> r;
	freeVars(set<Term*>(), a, r);
	return r;
}

void freeVars(Term* a, set<Term*> boundv, set<Term*>& freev) {
	switch (a->tag) {
	case All:
	case Exists:
		for (size_t i = 2; i < a->n; ++i) boundv.add(at(a, i));
		freeVars(at(a, 1), boundv, freev);
		return;
	case Var:
		if (boundv.count(a)) return;
		freev.add(a);
		return;
	}
	for (size_t i = 1; i < a->n; ++i) freeVars(at(a, i), boundv, freev);
}

Term* imp(Term* a, Term* b) {
	return mk(Or, mk(Not, a), b);
}

bool occurs(Term* a, Term* b) {
	if (a == b) return 1;
	for (size_t i = 1; i != b.size(); ++i)
		if (occurs(a, b[i])) return 1;
	return 0;
}

Term* quantify(Term* a) {
	auto vars = freeVars(a);
	if (vars.empty()) return a;
	vec<Term*> v(1, mk(All));
	v.push_back(a);
	for (auto x: vars) v.push_back(x);
	return mk(v);
}

// Are clauses sets of literals, or bags? It would seem logical to represent them as sets, and some algorithms prefer it that way,
// but unfortunately there are important algorithms that will break unless they are represented as bags, such as the superposition
// calculus:
// https://stackoverflow.com/questions/29164610/why-are-clauses-multisets
// So we represent them as bags (or lists, ignoring the order) and let the algorithms that prefer sets, discard duplicate literals
clause uniq(Clause* c) {
	vec<Term*> neg;
	for (auto& a: c.first)
		if (find(neg.begin(), neg.end(), a) == neg.end()) neg.push_back(a);

	vec<Term*> pos;
	for (auto& a: c.second)
		if (find(pos.begin(), pos.end(), a) == pos.end()) pos.push_back(a);

	return make_pair(neg, pos);
}
///
