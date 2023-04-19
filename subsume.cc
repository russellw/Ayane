#include "main.h"

// One clause subsumes another, if there exists a variable substitution that makes the first clause a sub-multiset of the second.
// Multiset not set, because otherwise a clause could subsume its own factors, which would break completeness of the superposition
// calculus.

// Improving the efficiency of this procedure is something of an open question:
// https://stackoverflow.com/questions/54043747/clause-subsumption-algorithm

// TODO: implement deterministic matching
// TODO: implement some form of indexing; can we do indexing on active clauses only?
namespace {
bool match(map<term, term>& m, vec<term> c, vec<term> d, vec<term> c2, vec<term> d2) {
	// Empty list means we have matched everything in one polarity. Note the asymmetry: For c to subsume d, we need to match every c
	// literal, but it's okay to have leftover d literals.
	if (c.empty()) {
		// Try the other polarity.
		if (c2.size()) return match(m, c2, d2, vec<term>(), vec<term>());

		// Nothing left to match in the other polarity.
		return 1;
	}

	// Try matching literals.
	for (size_t ci = 0; ci != c.size(); ++ci) {
		// Make an equation out of each literal, because an equation can be matched either way around.
		auto ce = eqn(c[ci]);

		// If we successfully match a literal, it can be removed from further consideration on this branch of the search tree. So
		// make a copy of this list of literals, minus the candidate lateral we are trying to match.
		auto c1 = c;
		c1.erase(c1.begin() + ci);
		for (size_t di = 0; di != d.size(); ++di) {
			// Same thing with the literals on the other side.
			auto de = eqn(d[di]);
			auto d1 = d;
			d1.erase(d1.begin() + di);

			// Remember where we were in case we need to backtrack.
			auto old = m;

			// Try orienting equation one way.
			if (match(m, ce.first, de.first) && match(m, ce.second, de.second)) {
				// If we successfully match this pair of literals, need to continue with the backtracking search, to see if these
				// variable assignments also let us match all the other literals.
				if (match(m, c1, d1, c2, d2)) return 1;
			}

			// Backtrack.
			m = old;

			// And the other way.
			if (match(m, ce.first, de.second) && match(m, ce.second, de.first)) {
				// And the rest of the search.
				if (match(m, c1, d1, c2, d2)) return 1;
			}

			// If this pair of literals did not match in either orientation of the respective equations, continue to look at all the
			// other possible pairs of literals.
		}
	}

	// Failed to find a way to match all the literals.
	return 0;
}
} // namespace

bool subsumes(const clause& c, const clause& d) {
	incStat("subsumes");

	// Negative and positive sides need to be matched separately, though of course with shared variable assignments.
	auto c1 = c.first;
	auto c2 = c.second;
	auto d1 = d.first;
	auto d2 = d.second;

	// It is impossible for a longer clause to subsume a shorter one.
	if (c1.size() > d1.size() || c2.size() > d2.size()) return 0;

	// Fewer literals are likely to fail faster, so if there are fewer positive literals than negative, swap them around and try the
	// positive side first.
	if (c2.size() < c1.size()) {
		swap(c1, c2);
		swap(d1, d2);
	}

	// Worst-case time is exponential. If the number of literals on the larger side of the subsuming clause exceeds a certain
	// threshold, make the pessimistic assumption that the check might take too long, and skip. It seems plausible that this will
	// not be much of a loss; apart from anything else, by the time we are trying to subsume with such large clauses, we are
	// probably in an unproductive part of the search space anyway.
	if (c2.size() > 10) {
		incStat("subsumes skipped");
		return 0;
	}

	// Search for matched literals.
	map<term, term> m;
	return match(m, c1, d1, c2, d2);
}

bool subsumesForward(const set<clause>& cs, const clause& d) {
	for (auto& c: cs)
		if (subsumes(c, d)) {
			incStat("subsumesForward");
			return 1;
		}
	return 0;
}

set<clause> subsumeBackward(const set<clause>& cs, const clause& d) {
	set<clause> r;
	for (auto& c: cs)
		if (subsumes(d, c)) incStat("subsumeBackward");
		else
			r.add(c);
	return r;
}
