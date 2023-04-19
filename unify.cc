#include "main.h"

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. This simplifies the data (representation of clauses) at the cost of making
// matching, unification and adjacent code more complex, which is usually a good trade.

// In particular, we cannot directly compare terms for equality (which in the normal course of events would indicate that two terms
// trivially unify) because two syntactically identical terms could be, or contain, the same variable names but with different
// associated subscripts.
static bool eq(term a, bool ax, term b, bool bx) {
	// If the terms are not syntactically equal then we definitely do not have logical equality.
	if (a != b) return 0;

	// If they are syntactically equal and on the same side then we definitely do have logical equality.
	if (ax == bx) return 1;

	// Two variables on different sides, are not equal.
	if (tag(a) == tag::Var) return 0;

	// Compound terms on opposite sides, even though syntactically equal, could contain variables, which would make them logically
	// unequal; to find out for sure, we would need to recur through subterms, but that is the job of match/unify, so here we just
	// give the conservative answer that they are not equal.
	if (a.size()) return 0;

	// Non-variable atoms do not have associated subscripts, so given that they are syntactically equal, they must be logically
	// equal.
	return 1;
}

bool match(map<term, term>& m, term a, term b) {
	// Equals.
	if (eq(a, 0, b, 1)) return 1;

	// Type mismatch.
	if (type(a) != type(b)) return 0;

	// Variable.
	// TODO: check variables more efficiently
	if (tag(a) == tag::Var) {
		auto& ma = m.gadd(a);

		// Existing mapping. First-order variables cannot be Boolean, which has the useful corollary that the default value of a
		// term (false) is distinguishable from any term to which a variable could be validly mapped.
		if (ma.raw) return ma == b;

		// New mapping.
		ma = b;
		return 1;
	}

	// Mismatched tags.
	// TODO: this step is not necessary?
	if (tag(a) != tag(b)) return 0;

	// If nonvariable atoms could match, they would already have tested equal.
	auto n = a.size();
	if (!n) return 0;

	// Recur.
	if (b.size() != n) return 0;
	if (a[0] != b[0]) return 0;
	for (size_t i = 1; i != n; ++i)
		if (!match(m, a[i], b[i])) return 0;
	return 1;
}

namespace {
bool occurs(const map<termx, termx>& m, term a, bool ax, term b, bool bx) {
	assert(tag(a) == tag::Var);
	if (tag(b) == tag::Var) {
		if (eq(a, ax, b, bx)) return 1;
		auto b1 = make_pair(b, bx);
		termx mb;
		if (m.get(b1, mb)) return occurs(m, a, ax, mb.first, mb.second);
	}
	for (size_t i = 1; i != b.size(); ++i)
		if (occurs(m, a, ax, b[i], bx)) return 1;
	return 0;
}

bool unifyVar(map<termx, termx>& m, term a, bool ax, term b, bool bx) {
	assert(tag(a) == tag::Var);
	assert(type(a) == type(b));

	// Existing mappings.
	auto a1 = make_pair(a, ax);
	termx ma;
	if (m.get(a1, ma)) return unify(m, ma.first, ma.second, b, bx);

	auto b1 = make_pair(b, bx);
	termx mb;
	if (m.get(b1, mb)) return unify(m, a, ax, mb.first, mb.second);

	// Occurs check.
	if (occurs(m, a, ax, b, bx)) return 0;

	// New mapping.
	m.add(a1, b1);
	return 1;
}
} // namespace

bool unify(map<termx, termx>& m, term a, bool ax, term b, bool bx) {
	// Equals.
	if (eq(a, ax, b, bx)) return 1;

	// Type mismatch.
	if (type(a) != type(b)) return 0;

	// Variable.
	if (tag(a) == tag::Var) return unifyVar(m, a, ax, b, bx);
	if (tag(b) == tag::Var) return unifyVar(m, b, bx, a, ax);

	// Mismatched tags.
	if (tag(a) != tag(b)) return 0;

	// If nonvariable atoms could unify, they would already have tested equal.
	auto n = a.size();
	if (!n) return 0;

	// Recur.
	if (b.size() != n) return 0;
	if (a[0] != b[0]) return 0;
	for (size_t i = 1; i != n; ++i)
		if (!unify(m, a[i], ax, b[i], bx)) return 0;
	return 1;
}

term replace(const map<termx, termx>& m, term a, bool ax) {
	auto a1 = make_pair(a, ax);
	termx ma;
	// TODO: check only if it is a variable
	if (m.get(a1, ma)) {
		assert(tag(a) == tag::Var);
		return replace(m, ma.first, ma.second);
	}

	auto n = a.size();
	vec<term> v(1, a[0]);
	for (size_t i = 1; i != n; ++i) v.push_back(replace(m, a[i], ax));
	return term(v);
}
