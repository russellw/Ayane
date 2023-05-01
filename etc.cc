#include "main.h"

const char* szsNames[] = {
#define _(x) #x,
#include "szs.h"
};

// SORT
Eqn eqn(Ex* a) {
	if (a->tag == Eq) return make_pair(at(a, 0), at(a, 2));
	return make_pair(a, atoms + True);
}

void flatten(int tag, Ex* a, std::vector<Ex*>& r) {
	if (a->tag == tag) {
		for (size_t i = 0; i < a->n; ++i) flatten(tag, at(a, i), r);
		return;
	}
	r.push_back(a);
}

void freeVars(Ex* a, vec<Ex*> boundv, vec<Ex*>& freev) {
	// TODO: boundv static?
	switch (a->tag) {
	case All:
	case Exists:
	{
		auto o = boundv.n;
		// TODO: batch add
		for (size_t i = 1; i < a->n; ++i) boundv.add(at(a, i));
		freeVars(at(a, 0), boundv, freev);
		boundv.n = o;
		return;
	}
	case Var:
		if (!boundv.has(a) && !freev.has(a)) freev.add(a);
		return;
	}
	for (size_t i = 0; i < a->n; ++i) freeVars(at(a, i), boundv, freev);
}

Ex* imp(Ex* a, Ex* b) {
	return ex(Or, ex(Not, a), b);
}

bool occurs(Ex* a, Ex* b) {
	if (a == b) return 1;
	for (size_t i = 0; i < b->n; ++i)
		if (occurs(a, at(b, i))) return 1;
	return 0;
}

Ex* quantify(Ex* a) {
	vec<Ex*> vars;
	freeVars(a, vec<Ex*>(), vars);
	if (vars.empty()) return a;
	vec<Ex*> v(1, a);
	// TODO: add all at once
	for (auto x: vars) v.add(x);
	return ex(All, v);
}
///
