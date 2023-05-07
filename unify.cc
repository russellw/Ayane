#include "main.h"

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. This simplifies the data (representation of clauses) at the cost of making
// matching, unification and adjacent code more complex, which is usually a good trade.

// In particular, we cannot directly compare terms for equality (which in the normal course of events would indicate that two terms
// trivially unify) because two syntactically identical terms could be, or contain, the same variable names but with different
// associated subscripts
// TODO: update terminology in comments
bool eq(ex a, bool ax, ex b, bool bx) {
	// If the terms are not syntactically equal then we definitely do not have logical equality
	if (a != b) return 0;

	// If they are syntactically equal and on the same side then we definitely do have logical equality
	if (ax == bx) return 1;

	// Two variables on different sides, are not equal
	if (a->tag == Var) return 0;

	// Compound terms on opposite sides, even though syntactically equal, could contain variables, which would make them logically
	// unequal; to find out for sure, we would need to recur through subterms, but that is the job of match/unify, so here we just
	// give the conservative answer that they are not equal
	if (a.size()) return 0;

	// Non-variable atoms do not have associated subscripts, so given that they are syntactically equal, they must be logically
	// equal
	return 1;
}

namespace {
vec<pair<exx, exx>> m;

bool occurs(ex a, bool ax, ex b, bool bx) {
	assert(a->tag == Var);
	if (b->tag == Var) {
		if (eq(a, ax, b, bx)) return 1;
		auto b1 = make_pair(b, bx);
		exx mb;
		if (m.get(b1, mb)) return occurs(a, ax, mb.first, mb.second);
	}
	for (size_t i = 0; i < b.size(); ++i)
		if (occurs(a, ax, b[i], bx)) return 1;
	return 0;
}

bool unifyVar(ex a, bool ax, ex b, bool bx) {
	assert(a->tag == Var);
	assert(type(a) == type(b));

	// Existing mappings
	auto a1 = make_pair(a, ax);
	exx ma;
	if (m.get(a1, ma)) return unify1(ma.first, ma.second, b, bx);

	auto b1 = make_pair(b, bx);
	exx mb;
	if (m.get(b1, mb)) return unify1(a, ax, mb.first, mb.second);

	// Occurs check
	if (occurs(a, ax, b, bx)) return 0;

	// New mapping
	m.add(a1, b1);
	return 1;
}

bool unify1(ex a, bool ax, ex b, bool bx) {
	// Equals
	if (eq(a, ax, b, bx)) return 1;

	// Type mismatch
	if (type(a) != type(b)) return 0;

	// Variable
	if (a->tag == Var) return unifyVar(a, ax, b, bx);
	if (b->tag == Var) return unifyVar(b, bx, a, ax);

	// Mismatched tags
	if (a->tag != b->tag) return 0;

	// If nonvariable atoms could unify, they would already have tested equal
	auto n = a.size();
	if (!n) return 0;

	// Recur
	if (b.size() != n) return 0;
	for (size_t i = 0; i < n; ++i)
		if (!unify1(at(a, i), ax, b[i], bx)) return 0;
	return 1;
}
} // namespace

bool unify(ex a, bool ax, ex b, bool bx) {
	m.n = 0;
	return unify1(a, ax, b, bx);
}

ex replace(ex a, bool ax) {
	auto a1 = make_pair(a, ax);
	exx ma;
	// TODO: check only if it is a variable
	if (m.get(a1, ma)) {
		assert(a->tag == Var);
		return replace(ma.first, ma.second);
	}

	auto n = a.size();
	vec<ex> v;
	for (size_t i = 0; i < n; ++i) v.add(replace(at(a, i), ax));
	return ex(a->tag, v);
}
