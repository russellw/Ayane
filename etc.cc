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
	r.add(a);
}

vec<Term*> flatten(int tag, Term* a) {
	vec<Term*> r;
	flatten(t, a, r);
	return r;
}

void freeVars(Term* a, vec<Term*> boundv, vec<Term*>& freev) {
	// TODO: boundv static?
	switch (a->tag) {
	case All:
	case Exists:
	{
		auto o = boundv.n;
		// TODO: batch add
		for (size_t i = 2; i < a->n; ++i) boundv.add(at(a, i));
		freeVars(at(a, 1), boundv, freev);
		boundv.n = o;
		return;
	}
	case Var:
		if (!boundv.has(a) && !freev.has(a)) freev.add(a);
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
	v.add(a);
	for (auto x: vars) v.add(x);
	return mk(v);
}
///
