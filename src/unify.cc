#include "main.h"

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. This simplifies the data (representation of clauses) at the cost of making
// matching, unification and adjacent code more complex, which is usually a good trade.

// In particular, we cannot directly compare terms for equality (which in the normal course of events would indicate that two terms
// trivially unify) because two syntactically identical terms could be, or contain, the same variable names but with different
// associated subscripts
// TODO: update terminology in comments
bool eq(Expr* a, bool ax, Expr* b, bool bx) {
	// If the terms are not syntactically equal then we definitely do not have logical equality
	if (a != b) return 0;

	// If they are syntactically equal and on the same side then we definitely do have logical equality
	if (ax == bx) return 1;

	// Two variables on different sides, are not equal
	if (a->tag == Tag::var) return 0;

	// Composite terms on opposite sides, even though syntactically equal, could contain variables, which would make them logically
	// unequal; to find out for sure, we would need to recur through subterms, but that is the job of match/unify, so here we just
	// give the conservative answer that they are not equal
	if (a->n) return 0;

	// Non-variable leaves do not have associated subscripts, so given that they are syntactically equal, they must be logically
	// equal
	return 1;
}

namespace {
Vec<pair<pair<Var*, bool>, pair<Expr*, bool>>> m;

bool occurs(Var* a, bool ax, Expr* b, bool bx) {
	assert(a->tag == Tag::var);
	if (b->tag == Tag::var) {
		if (eq(a, ax, b, bx)) return 1;
		pair<Expr*, bool> r;
		if (get(make_pair((Var*)b, bx), r, m)) return occurs(a, ax, r.first, r.second);
	}
	for (auto bi: b)
		if (occurs(a, ax, bi, bx)) return 1;
	return 0;
}

bool unify1(Expr* a, bool ax, Expr* b, bool bx);

bool unifyVar(Var* a, bool ax, Expr* b, bool bx) {
	assert(a->tag == Tag::var);
	assert(type(a) == type(b));

	// Existing mappings
	pair<Expr*, bool> r;
	if (get(make_pair(a, ax), r, m)) return unify1(r.first, r.second, b, bx);
	if (b->tag == Tag::var && get(make_pair((Var*)b, bx), r, m)) return unify1(a, ax, r.first, r.second);

	// Occurs check
	if (occurs(a, ax, b, bx)) return 0;

	// New mapping
	m.add(make_pair(make_pair(a, ax), make_pair(b, bx)));
	return 1;
}

bool unify1(Expr* a, bool ax, Expr* b, bool bx) {
	// Equals
	if (eq(a, ax, b, bx)) return 1;

	// Type mismatch
	if (type(a) != type(b)) return 0;

	// Variable
	if (a->tag == Tag::var) return unifyVar((Var*)a, ax, b, bx);
	if (b->tag == Tag::var) return unifyVar((Var*)b, bx, a, ax);

	// Different operators
	if (a->tag != b->tag) return 0;

	// If nonvariable atoms could unify, they would already have tested equal
	auto n = a->n;
	if (!n) return 0;

	// Recur
	if (b->n != n) return 0;
	for (size_t i = 0; i < n; ++i)
		if (!unify1(at(a, i), ax, at(b, i), bx)) return 0;
	return 1;
}
} // namespace

bool unify(Expr* a, bool ax, Expr* b, bool bx) {
	m.n = 0;
	return unify1(a, ax, b, bx);
}

Expr* replace(Expr* a, bool ax) {
	if (a->tag == Tag::var) {
		pair<Expr*, bool> r;
		if (get(make_pair((Var*)a, ax), r, m)) return replace(r.first, r.second);
	}
	if (!a->n) return a;
	Vec<Expr*> v;
	for (auto b: a) v.add(replace(b, ax));
	return comp(a->tag, v);
}
