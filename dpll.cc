#include "main.h"

szs dpll(map<term, term>& m, const set<clause>& cs) {
	auto cs1 = uniq(cs);
	cs1 = simplify(m, cs1);

	// If we have all true clauses, satisfiable; as simplify filters out tautologies, this amounts to a check that no clauses remain
	// unsatisfied.
	if (cs1.empty()) return szs::Satisfiable;

	// If we have any false clause, not satisfiable.
	if (cs1.count(falsec)) return szs::Unsatisfiable;

	// Unit clauses.
	for (auto& c: cs1)
		if (c.first.size() + c.second.size() == 1) {
			if (c.first.size()) m.add(c.first[0], tag::False);
			else
				m.add(c.second[0], tag::True);
			return dpll(m, cs);
		}

	// Atoms.
	set<term> atoms;
	for (auto& c: cs1) {
		atoms.add(c.first.begin(), c.first.end());
		atoms.add(c.second.begin(), c.second.end());
	}

	// Choose an arbitrary atom.
	auto& a = *atoms.begin();

	// Try assigning false.
	auto m1 = m;
	m1.add(a, tag::False);
	if (dpll(m1, cs1) == szs::Satisfiable) {
		m = m1;
		return szs::Satisfiable;
	}

	// Try assigning true.
	m1 = m;
	m1.add(a, tag::True);
	if (dpll(m1, cs1) == szs::Satisfiable) {
		m = m1;
		return szs::Satisfiable;
	}

	// Fail.
	return szs::Unsatisfiable;
}
