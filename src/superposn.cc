#include "main.h"

namespace {
bool isNumeric(term a) {
	switch (kind(type(a))) {
	case kind::Integer:
	case kind::Rational:
	case kind::Real:
		return 1;
	}
	return 0;
}

bool hasNumeric(term a) {
	if (isNumeric(a)) return 1;
	for (size_t i = 1; i != a.size(); ++i)
		if (hasNumeric(a[i])) return 1;
	return 0;
}

bool hasNumeric(clause c) {
	for (auto a: c.first)
		if (hasNumeric(a)) return 1;
	for (auto a: c.second)
		if (hasNumeric(a)) return 1;
	return 0;
}

term splice(term a, const vec<size_t>& posn, size_t i, term b) {
	if (i == posn.size()) return b;

	auto n = a.size();
	vec<term> v(n);
	for (size_t j = 0; j != n; ++j) {
		v[j] = a[j];
		if (j == posn[i]) v[j] = splice(v[j], posn, i + 1, b);
	}
	return term(v);
}

// Passive clauses are stored in a priority queue with smaller clauses first.
size_t weight(term a) {
	size_t n = 1;
	for (size_t i = 1; i < a.size(); ++i) n += weight(a[i]);
	return n;
}

size_t weight(const clause& c) {
	size_t n = 0;
	for (auto a: c.first) n += weight(a);
	for (auto a: c.second) n += weight(a);
	return n;
}

struct cmpc {
	bool operator()(const clause& c, const clause& d) {
		return weight(c) > weight(d);
	}
};

// First-order logic usually takes the view that equality is a special case, but superposition calculus takes the view that equality
// is the general case. Non-equality predicates are considered to be equations 'p=true'; this is a special exemption from the usual
// rule that equality is not allowed on Boolean terms.
bool equatable(term a, term b) {
	if (type(a) != type(b)) return 0;
	if (type(a) == kind::Bool) return a == tag::True || b == tag::True;
	return 1;
}

term equate(term a, term b) {
	assert(equatable(a, b));
	if (a == tag::True) return b;
	if (b == tag::True) return a;
	assert(type(a) != kind::Bool);
	assert(type(b) != kind::Bool);
	return term(tag::Eq, a, b);
}

// Equality tends to generate a large number of clauses. Superposition calculus is designed to moderate the profusion of clauses
// using an ordering on terms, that tries to apply equations in one direction only; the difficulty, of course, is doing this without
// breaking completeness.
class lexicographicPathOrder {
	// The greater-than test is supposed to be called on complete terms, which can include constant symbols (zero arity), or calls
	// of function symbols (positive arity) with arguments. Make sure it's not being called on an isolated function symbol.
	void check(term a) {
		assert(kind(type(a)) != kind::Fn);
	}

	bool ge(term a, term b) {
		return a == b || gt(a, b);
	}

public:
	lexicographicPathOrder(const set<clause>& cs) {
		// At this point, should preferably do some analysis on the operators used in the clauses, to figure out what order is
		// likely to be best. For now, just use an arbitrary order.
	}

	// Check whether one term is unambiguously greater than another. This is much more delicate than comparison for e.g. sorting,
	// where arbitrary choices can be made; to avoid breaking completeness of the calculus, the criteria are much stricter, and when
	// in doubt, we return false.
	bool gt(term a, term b) {
		check(a);
		check(b);

		// Fast equality test.
		if (a == b) return 0;

		// Variables are unordered unless contained in other term.
		// TODO: check how that relates to variable identity between clauses
		if (tag(a) == tag::Var) return 0;
		if (tag(b) == tag::Var) return occurs(b, a);

		// Sufficient condition: Exists ai >= b.
		for (size_t i = 1; i < a.size(); i++)
			if (ge(a[i], b)) return 1;

		// Necessary condition: a > all bi.
		for (size_t i = 1; i < b.size(); i++)
			if (!gt(a, b[i])) return 0;

		// Different operators. Comparison by atom offset has the required property that True is considered smaller than any other
		// term (except False, which does not occur during superposition proof search).
		auto c = (int)a.getAtomOffset() - (int)b.getAtomOffset();
		if (c) return c > 0;

		// Same operators should mean similar terms.
		assert(tag(a) == tag(b));
		assert(a.size() == b.size());
		assert(a[0] == b[0]);

		// Lexicographic extension.
		for (size_t i = 1; i < a.size(); i++) {
			if (gt(a[i], b[i])) return 1;
			if (a[i] != b[i]) return 0;
		}

		// Having found no differences, the terms must be equal, but we already checked for that first thing, so something is wrong.
		unreachable;
	}
};

struct doing {
	// SORT
	int mode;
	lexicographicPathOrder order;
	std::priority_queue<clause, vec<clause>, cmpc> passive;
	Proof& proof;
	///

	// If a complete saturation proof procedure finds no more possible derivations, then the problem is satisfiable. In practice,
	// this almost never happens for nontrivial problems, but serves as a good way to test the completeness of the prover on some
	// trivial problems. However, if completeness was lost for any reason, then we will need to report failure instead.
	szs result = szs::Satisfiable;

	void qclause(rule rl, const vec<clause>& from, const vec<term>& neg, const vec<term>& pos) {
		incStat("qclause");
		auto c = make_pair(neg, pos);

		// Immediately simplifying clauses is efficient, and seems like it should not break completeness, though it would be nice to
		// see an actual proof of this:
		// https://stackoverflow.com/questions/65162921/is-superposition-complete-with-immediate-simplify
		c = simplify(map<term, term>(), c);

		// Filter tautologies.
		if (c == truec) {
			incStat("qclause filter tautology");
			return;
		}

		// We need to keep track of the proof anyway for output later, so it might as well do double duty as a record of which
		// clauses we have already seen, for filtering duplicates.
		if (proof.count(c)) {
			incStat("qclause filter dup");
			return;
		}

		// Record proof step.
		proof.add(c, make_pair(rl, from));

		// Add to the passive queue.
		incStat("qclause add");
		passive.push(c);
	}

	/*
	equality resolution
		c | c0 != c1
	->
		c/s
	where
		s = unify(c0, c1)
	*/

	// Check, substitute and make new clause.
	void resolve(const clause& c, size_t ci, term c0, term c1) {
		// Unify.
		map<termx, termx> m;
		if (!unify(m, c0, 0, c1, 0)) return;

		// Negative literals.
		vec<term> neg;
		for (size_t i = 0; i != c.first.size(); ++i)
			if (i != ci) neg.push_back(replace(m, c.first[i], 0));

		// Positive literals.
		vec<term> pos;
		for (size_t i = 0; i != c.second.size(); ++i) pos.push_back(replace(m, c.second[i], 0));

		// Make new clause.
		qclause(rule::er, vec<clause>{c}, neg, pos);
	}

	// For each negative equation.
	void resolve(const clause& c) {
		for (size_t ci = 0; ci != c.first.size(); ++ci) {
			auto e = eqn(c.first[ci]);
			resolve(c, ci, e.first, e.second);
		}
	}

	/*
	equality factoring
		c | c0 = c1 | d0 = d1
	->
		(c | c0 = c1 | c1 != d1)/s
	where
		s = unify(c0, d0)
	*/

	// Check, substitute and make new clause.
	void factor(const clause& c, size_t ci, term c0, term c1, size_t di, term d0, term d1) {
		// If these two terms are not equatable (for which the types must match, and predicates can only be equated with True),
		// substituting terms for variables would not make them become so.
		if (!equatable(c1, d1)) return;

		// Unify.
		map<termx, termx> m;
		if (!unify(m, c0, 0, d0, 0)) return;

		// Negative literals.
		vec<term> neg;
		for (size_t i = 0; i != c.first.size(); ++i) neg.push_back(replace(m, c.first[i], 0));
		neg.push_back(equate(replace(m, c1, 0), replace(m, d1, 0)));

		// Positive literals.
		vec<term> pos;
		for (size_t i = 0; i != c.second.size(); ++i)
			if (i != di) pos.push_back(replace(m, c.second[i], 0));

		// Make new clause.
		qclause(rule::ef, vec<clause>{c}, neg, pos);
	}

	// For each positive equation (both directions) again.
	void factor(const clause& c, size_t ci, term c0, term c1) {
		for (size_t di = 0; di != c.second.size(); ++di) {
			if (di == ci) continue;
			auto e = eqn(c.second[di]);
			factor(c, ci, c0, c1, di, e.first, e.second);
			factor(c, ci, c0, c1, di, e.second, e.first);
		}
	}

	// For each positive equation (both directions).
	void factor(const clause& c) {
		for (size_t ci = 0; ci != c.second.size(); ++ci) {
			auto e = eqn(c.second[ci]);
			factor(c, ci, e.first, e.second);
			factor(c, ci, e.second, e.first);
		}
	}

	/*
	superposition
		c | c0 = c1, d | d0(a) ?= d1
	->
		(c | d | d0(c1) ?= d1)/s
	where
		s = unify(c0, a)
		a is not a variable
	*/

	// The literature describes negative and positive superposition as separate inference rules; the only difference between them is
	// whether they consider negative or positive equations in the second clause, so to avoid copy-pasting a significant chunk of
	// nontrivial and almost identical code, we specify here a single inference rule controlled by the mode flag.

	// Check, substitute and make new clause.
	void superposn(
		const clause& c,
		const clause& d,
		size_t ci,
		term c0,
		term c1,
		size_t di,
		term d0,
		term d1,
		const vec<size_t>& posn,
		term a) {
		// It is never necessary to paramodulate into variables.
		if (tag(a) == tag::Var) return;

		// Unify.
		map<termx, termx> m;
		if (!unify(m, c0, 0, a, 1)) return;

		// Negative literals.
		vec<term> neg;
		for (size_t i = 0; i != c.first.size(); ++i) neg.push_back(replace(m, c.first[i], 0));
		for (size_t i = 0; i != d.first.size(); ++i) {
			if (!mode && i == di) continue;
			neg.push_back(replace(m, d.first[i], 1));
		}

		// Positive literals.
		vec<term> pos;
		for (size_t i = 0; i != c.second.size(); ++i)
			if (i != ci) pos.push_back(replace(m, c.second[i], 0));
		for (size_t i = 0; i != d.second.size(); ++i) {
			if (mode && i == di) continue;
			pos.push_back(replace(m, d.second[i], 1));
		}

		// To calculate d0(c1), we first perform the replacement of variables with substitute values, on the component terms, then
		// splice them together. This is necessary because the component terms are from different clauses, therefore have different
		// logical variable names. The composition would not be valid if we were replacing arbitrary terms, but is valid because we
		// are only replacing variables.
		d0 = replace(m, d0, 1);
		c1 = replace(m, c1, 0);
		auto d0c1 = splice(d0, posn, 0, c1);
		d1 = replace(m, d1, 1);

		// Make equation.
		if (!equatable(d0c1, d1)) return;
		auto& v = mode ? pos : neg;
		v.push_back(equate(d0c1, d1));

		// Make new clause.
		qclause(rule::sp, vec<clause>{c, d}, neg, pos);
	}

	// Descend into subterms.
	void descend(
		const clause& c,
		const clause& d,
		size_t ci,
		term c0,
		term c1,
		size_t di,
		term d0,
		term d1,
		const vec<size_t>& posn,
		term a) {
		superposn(c, d, ci, c0, c1, di, d0, d1, posn, a);
		for (size_t i = 1; i < a.size(); ++i) {
			auto p(posn);
			p.push_back(i);
			descend(c, d, ci, c0, c1, di, d0, d1, p, a[i]);
		}
	}

	// For each (mode)ve equation in d (both directions).
	void superposn(const clause& c, const clause& d, size_t ci, term c0, term c1) {
		auto& dmode = mode ? d.second : d.first;
		for (size_t di = 0; di != dmode.size(); ++di) {
			auto e = eqn(dmode[di]);
			descend(c, d, ci, c0, c1, di, e.first, e.second, vec<size_t>(), e.first);
			descend(c, d, ci, c0, c1, di, e.second, e.first, vec<size_t>(), e.second);
		}
	}

	// For each positive equation in c (both directions).
	void superposn(const clause& c, const clause& d) {
		for (size_t ci = 0; ci != c.second.size(); ++ci) {
			auto e = eqn(c.second[ci]);
			superposn(c, d, ci, e.first, e.second);
			superposn(c, d, ci, e.second, e.first);
		}
	}

	doing(const set<clause>& cs, Proof& proof, uint64_t iterLimit): order(cs), proof(proof) {
		// First-order logic is not complete on arithmetic. The conservative approach to this is that if any clause (after
		// simplification, which includes evaluation of ground terms) contains terms of numeric type, we mark the proof search
		// incomplete, so that failure to derive a contradiction, means the result is inconclusive rather than satisfiable.
		for (auto c: cs)
			if (hasNumeric(simplify(map<term, term>(), c))) {
				result = szs::GaveUp;
				break;
			}

		// The passive queue starts off with all the input clauses.
		for (auto& c: cs) qclause(rule::cnf, vec<clause>(), c.first, c.second);

		// The active set starts off empty.
		set<clause> active;

		// Saturation proof procedure tries to perform all possible derivations until it derives a contradiction.
		for (uint64_t i = 0; i != iterLimit; ++i) {
			// If there are no more clauses in the queue, the problem is satisfiable, unless completeness was lost.
			if (passive.empty()) return;
			incStat("superposn main loop");

			// Given clause.
			auto g = passive.top();
			passive.pop();

			// Derived false.
			if (g == falsec) {
				result = szs::Unsatisfiable;
				return;
			}

			// This is the Discount loop (in which only active clauses participate in subsumption checks); in tests, it performed
			// slightly better than the alternative Otter loop (in which passive clauses also participate).
			if (subsumesForward(active, g)) continue;
			active = subsumeBackward(active, g);

			// Add g to active clauses before inference, because we will sometimes need to combine g with itself.
			active.add(g);

			// Infer.
			resolve(g);
			factor(g);
			for (auto& c: active)
				for (mode = 0; mode != 2; ++mode) {
					superposn(c, g);
					superposn(g, c);
				}
		}
		result = szs::Timeout;
	}
};
} // namespace

szs superposn(const set<clause>& cs, Proof& proof, uint64_t iterLimit) {
	doing _(cs, proof, iterLimit);
	return _.result;
}
