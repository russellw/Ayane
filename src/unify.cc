#include "main.h"

namespace {
Vec<pair<Var*, Expr*>> m;

bool occurs(Var* a, Expr* b) {
	assert(a->tag == Tag::var);
	if (b->tag == Tag::var) {
		if (a == b) return 1;
		Expr* bm;
		if (get((Var*)b, bm, m)) return occurs(a, bm);
	}
	for (auto bi: b)
		if (occurs(a, bi)) return 1;
	return 0;
}

bool unify1(Expr* a, Expr* b);

bool unifyVar(Var* a, Expr* b) {
	assert(a->tag == Tag::var);
	assert(type(a) == type(b));

	// Existing mappings
	Expr* y;
	if (get(a, y, m)) return unify1(y, b);
	if (b->tag == Tag::var && get((Var*)b, y, m)) return unify1(a, y);

	// Occurs check
	if (occurs(a, b)) return 0;

	// New mapping
	m.add(make_pair(a, b));
	return 1;
}

bool unify1(Expr* a, Expr* b) {
	// Equals
	if (a == b) return 1;

	// Type mismatch
	if (type(a) != type(b)) return 0;

	// Variable
	if (a->tag == Tag::var) return unifyVar((Var*)a, b);
	if (b->tag == Tag::var) return unifyVar((Var*)b, a);

	// Different operators
	if (a->tag != b->tag) return 0;

	// If nonvariable leaves could unify, they would already have tested equal
	if (!a->n) return 0;

	// Recur
	if (a->n != b->n) return 0;
	for (size_t i = 0; i < a->n; ++i)
		if (!unify1(at(a, i), at(b, i))) return 0;
	return 1;
}
} // namespace

bool unify(Expr* a, Expr* b) {
	m.n = 0;
	return unify1(a, b);
}

Expr* replace(Expr* a) {
	// Variable
	if (a->tag == Tag::var) {
		Expr* am;
		if (get((Var*)a, am, m)) return replace(am);
	}

	// Leaf
	if (!a->n) return a;

	// Composite
	Vec<Expr*> v(a->n);
	for (size_t i = 0; i < a->n; ++i) v[i] = replace(at(a, i));
	return comp(a->tag, v);
}
