#include "main.h"

namespace {
const size_t many = 50;

// How many clauses a term will expand into, for the purpose of deciding when subformulas need to be renamed. The answer could
// exceed the range of a fixed-size integer, but then we don't actually need the number, we only need to know whether it went over
// the threshold.
size_t ncs(bool pol, Term* a);

size_t ncsMul(bool pol, Term* a) {
	size_t n = 1;
	for (size_t i = 1; i < a->n; ++i) {
		n *= ncs(pol, at(a, i));
		if (n >= many) return many;
	}
	return n;
}

size_t ncsAdd(bool pol, Term* a) {
	size_t n = 0;
	for (size_t i = 1; i < a->n; ++i) {
		n += ncs(pol, at(a, i));
		if (n >= many) return many;
	}
	return n;
}

size_t ncs(bool pol, Term* a) {
	// TODO: really want NO_SORT?
	// NO_SORT
	switch (a->tag) {
	case All:
	case Exists:
		return ncs(pol, at(a, 1));

	case Not:
		return ncs(!pol, at(a, 1));

	case Or:
		return pol ? ncsMul(pol, a) : ncsAdd(pol, a);
	case And:
		return pol ? ncsAdd(pol, a) : ncsMul(pol, a);

	case Eqv:
	{
		auto x = at(a, 1);
		auto y = at(a, 2);

		// Recur twice into each argument. This would cause a problem of exponential blowup in the time taken to calculate the
		// number of clauses that would be generated by nested equivalences. We solve this problem by returning early if the number
		// is becoming large.
		size_t n;
		if (pol) {
			n = ncs(0, x) * ncs(1, y);
			if (n >= many) return many;
			n += ncs(1, x) * ncs(0, y);
		} else {
			n = ncs(0, x) * ncs(0, y);
			if (n >= many) return many;
			n += ncs(1, x) * ncs(1, y);
		}
		return min(n, many);
	}
	}
	return 1;
}

// The function to calculate the number of clauses generated by a formula in a positive or negative context, returns a
// mathematically defined result (up to a ceiling). However, when it comes to actually trying to rename formulas, we may be dealing
// with both positive and negative contexts. In particular, this may happen in nested equivalences, where the total number of
// formulas cannot be calculated without the full context, but would in any case be unreasonably large, so it is neither feasible
// nor necessary to calculate the number. What we actually need to do is make a heuristic decision. To that end, if we have a
// context that is both positive and negative, we add the two values for the number of clauses; this doesn't have a clear
// mathematical justification, but seems as reasonable as anything else, and simple enough that there are hopefully few ways it can
// go wrong.
size_t ncsApprox(int pol, Term* a) {
	size_t n = 0;
	if (pol >= 0) n += ncs(1, a);
	if (pol <= 0) n += ncs(0, a);
	return n;
}

// Skolem functions replace existentially quantified variables, also formulas that are renamed to avoid exponential expansion
Term* skolem(type rty, const vec<Term*>& args) {
	vec<Term*> v(1, gensym(ftype(rty, args)));
	// TODO: single call instead of loop?
	for (auto b: args) v.add(b);
	return mk(v);
}

// SORT
vec<Term*> defs;
size_t vars = 0;
///

// Rename formulas to avoid exponential expansion. It's tricky to do this while in the middle of doing other things, easier to be
// sure of the logic if it's done as a separate pass first.
Term* rename(int pol, Term* a) {
	vec<Term*> freev;
	freeVars(a, vec<Term*>(), freev);
	auto b = skolem(getType(a), freev);
	// NO_SORT
	switch (pol) {
	case 1:
		// If this formula is only being used with positive polarity, the new name only needs to imply the original formula
		a = imp(b, a);
		break;
	case -1:
		// And the reverse for negative polarity
		a = imp(a, b);
		break;
	case 0:
		// In the general case, full equivalence is needed; the new name implies and is implied by the original formula
		a = mk(And, imp(b, a), imp(a, b));
		break;
	default:
		unreachable;
	}
	defs.add(quantify(a));
	return b;
}

// Maybe rename some of the arguments to an OR-over-AND (taking polarity into account), where the number of clauses generated would
// be the product of the arguments
void maybeRename(int pol, vec<Term*>& v) {
	// Sorting the arguments doesn't change the meaning of the formula, because AND and OR are commutative. The effect is that if
	// only some of them are to be renamed, we will leave the simple ones alone and end up renaming the complicated ones, which is
	// probably what we want.
	sort(v.begin() + 1, v.end(), [=](Term* a, Term* b) { return ncsApprox(pol, a) < ncsApprox(pol, b); });
	size_t n = 1;
	for (size_t i = 1; i != v.size(); ++i) {
		auto m = ncsApprox(pol, v[i]);
		if (n * m < many) n *= m;
		else
			v[i] = rename(pol, v[i]);
	}
}

// Given a formula, and whether it is used for positive polarity, negative or both (i.e. under an equivalence), maybe rename some of
// its subformulas. If a subformula occurs many times (whether under the same formula, or different ones), it is considered in
// isolation each time, so that each occurrence could end up with a different name. In principle, it would be more efficient to
// rename on a global basis, but in practice, nontrivial subformulas are rarely duplicated (e.g. less than 1% of the nontrivial
// formulas in the TPTP), so this is probably not worth doing.
Term* maybeRename(int pol, Term* a) {
	vec<Term*> v(1, at(a, 0));
	// NO_SORT
	switch (a->tag) {
	case All:
	case Exists:
		v.add(maybeRename(pol, at(a, 1)));
		for (size_t i = 2; i < a->n; ++i) v.add(at(a, i));
		break;

	case Not:
		return mk(Not, maybeRename(-pol, at(a, 1)));

	case Or:
		for (size_t i = 1; i < a->n; ++i) v.add(maybeRename(pol, at(a, i)));

		// If this formula will be used with positive polarity (including the case where it will be used both ways), we are looking
		// at OR over possible ANDs, which would produce exponential expansion at the distribution stage, so may need to rename some
		// of the arguments
		if (pol >= 0) maybeRename(pol, v);
		break;
	case And:
		for (size_t i = 1; i < a->n; ++i) v.add(maybeRename(pol, at(a, i)));

		// NOT-AND yields OR, so mirror the OR case
		if (pol <= 0) maybeRename(pol, v);
		break;

	case Eqv:
	{
		auto x = maybeRename(0, at(a, 1));
		auto y = maybeRename(0, at(a, 2));
		if (ncsApprox(0, x) >= many) x = rename(0, x);
		if (ncsApprox(0, y) >= many) y = rename(0, y);
		return mk(Eqv, x, y);
	}

	default:
		return a;
	}
	return mk(v);
}

Term* nnf(bool pol, Term* a, vec<pair<Term*, Term*>>& m);

// For-all doesn't need much work to convert. Clauses contain variables with implied for-all. The tricky part is that quantifier
// binds variables to local scope, so the same variable name used in two for-all's corresponds to two different logical variables.
// So we rename each quantified variable to a new variable of the same type.
Term* all(int pol, Term* a, vec<pair<Term*, Term*>>& m) {
	auto o = m.n;
	for (size_t i = 2; i < a->n; ++i) {
		auto x = at(a, i);
		assert(x->tag == Var);
		auto y = var(vars++, ((Atom*)x)->ty);
		m.add(make_pair(x, y));
	}
	a = nnf(pol, at(a, 1), m);
	m.n = o;
	return a;
}

// Each existentially quantified variable is replaced with a Skolem function whose parameters are all the surrounding universally
// quantified variables
Term* exists(int pol, Term* a, vec<pair<Term*, Term*>>& m) {
	// Get the surrounding universally quantified variables that will be arguments to the Skolem functions
	vec<Term*> args;
	for (auto& xy: m)
		if (xy.second->tag == Var) args.add(xy.second);

	// Make a replacement for each existentially quantified variable
	auto o = m.n;
	for (size_t i = 2; i < a->n; ++i) {
		auto x = at(a, i);
		assert(x->tag == Var);
		auto y = skolem(((Atom*)x)->ty, args);
		m.add(make_pair(x, y));
	}
	a = nnf(pol, at(a, 1), m);
	m.n = o;
	return a;
}

// Negation normal form consists of several transformations that are as easy to do at the same time: Move NOTs inward to the literal
// layer, flipping things around on the way, while simultaneously resolving quantifiers
Term* nnf(bool pol, Term* a, vec<pair<Term*, Term*>>& m) {
	vec<Term*> v(1, at(a, 0));
	// NO_SORT
	switch (a->tag) {
	case False:
		// Boolean constants and operators can be inverted by downward-sinking NOTs
		return mkbool(!pol);
	case True:
		return mkbool(pol);

	case Not:
		return nnf(!pol, at(a, 1), m);

	case Or:
		if (!pol) v[0] = mk(And);
		for (size_t i = 1; i < a->n; ++i) v.add(nnf(pol, at(a, i), m));
		return mk(v);
	case And:
		if (!pol) v[0] = mk(Or);
		for (size_t i = 1; i < a->n; ++i) v.add(nnf(pol, at(a, i), m));
		return mk(v);

		// Variables are mapped to new variables or Skolem functions
	case Var:
		return m.at(a);

		// According to whether they are bound by universal or existential quantifiers
	case All:
		m = pol ? all(m, a) : exists(m, a);
		return nnf(m, pol, at(a, 1));
	case Exists:
		m = pol ? exists(m, a) : all(m, a);
		return nnf(m, pol, at(a, 1));

		// Equivalence is the most difficult operator to deal with
	case Eqv:
	{
		auto x = at(a, 1);
		auto y = at(a, 2);
		auto x0 = nnf(m, 0, x);
		auto x1 = nnf(m, 1, x);
		auto y0 = nnf(m, 0, y);
		auto y1 = nnf(m, 1, y);
		return pol ? mk(And, mk(Or, x0, y1), mk(Or, x1, y0)) : mk(And, mk(Or, x0, y0), mk(Or, x1, y1));
	}
	}
	for (size_t i = 1; i < a->n; ++i) v.add(nnf(m, 1, at(a, i)));
	a = mk(v);
	return pol ? a : mk(Not, a);
}

// Distribute OR down into AND, completing the layering of the operators for CNF. This is the second place where exponential
// expansion would occur, had selected formulas not already been renamed.
Term* distribute(Term* a) {
	vec<Term*> v(1, mk(And));
	switch (a->tag) {
	case And:
		for (size_t i = 1; i < a->n; ++i) v.add(distribute(at(a, i)));
		break;
	case Or:
	{
		// Arguments can be taken without loss of generality as ANDs
		vec<vec<Term*>> ands;
		for (size_t i = 1; i < a->n; ++i) {
			// Recur
			auto b = distribute(at(a, i));

			// And make a flat layer of ANDs
			ands.add(flatten(And, b));
		}

		// OR distributes over AND by Cartesian product
		// TODO: can this be done by reference?
		for (auto u: cartProduct(ands)) {
			u.insert(u.begin(), mk(Or));
			v.add(mk(u));
		}
		break;
	}
	default:
		return a;
	}
	return mk(v);
}

// Convert a suitably rearranged term into actual clauses
void clauseTerm(Term* a, vec<Term*>& neg, vec<Term*>& pos) {
	switch (a->tag) {
	case All:
	case And:
	case Eqv:
	case Exists:
		unreachable;
	case Not:
		neg.add(at(a, 1));
		return;
	case Or:
		for (size_t i = 1; i < a->n; ++i) clauseTerm(at(a, i), neg, pos);
		return;
	}
	pos.add(a);
}

clause clauseTerm(Term* a) {
	vec<Term*> neg, pos;
	clauseTerm(a, neg, pos);
	auto c = make_pair(neg, pos);
	c = simplify(map<Term*, Term*>(), c);
	return c;
}

// And record the clauses
void csTerm(Term* from, Term* a) {
	for (auto& b: flatten(And, a)) {
		auto c = clauseTerm(b);
		if (c == truec) continue;
		proofCnf.add(c, from);
		cs.add(c);
	}
}
} // namespace

void cnf(Formula* formula) {
	// First run each input formula through the full process: Rename subformulas where necessary to avoid exponential expansion,
	// then convert to negation normal form, distribute OR into AND, and convert to clauses
	for (auto a: initialFormulas) {
		auto from = a;
		a = maybeRename(1, a);
		vars = 0;
		a = nnf(map<Term*, Term*>(), 1, a);
		a = distribute(a);
		csTerm(from, a);
	}

	// Then convert all the definitions created by the renaming process. That process works by bottom-up recursion, which means each
	// renamed subformula is simple, so there is no need to put the definitions through the renaming process again; they just need
	// to go through the rest of the conversion steps.
	for (auto a: defs) {
		auto from = a;
		vars = 0;
		a = nnf(map<Term*, Term*>(), 1, a);
		a = distribute(a);
		csTerm(from, a);
	}
}
