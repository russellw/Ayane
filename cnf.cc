#include "main.h"

namespace {
const size_t many = 50;

// How many clauses a term will expand into, for the purpose of deciding when subformulas need to be renamed. The answer could
// exceed the range of a fixed-size integer, but then we don't actually need the number, we only need to know whether it went over
// the threshold.
size_t ncs(bool pol, Ex* a);

size_t ncsMul(bool pol, Ex* a) {
	size_t n = 1;
	for (size_t i = 0; i < a->n; ++i) {
		n *= ncs(pol, at(a, i));
		if (n >= many) return many;
	}
	return n;
}

size_t ncsAdd(bool pol, Ex* a) {
	size_t n = 0;
	for (size_t i = 0; i < a->n; ++i) {
		n += ncs(pol, at(a, i));
		if (n >= many) return many;
	}
	return n;
}

size_t ncs(bool pol, Ex* a) {
	// TODO: really want NO_SORT?
	// NO_SORT
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
		return ncs(pol, at(a, 0));

	case Tag::not:
		return ncs(!pol, at(a, 0));

	case Tag:: or:
		return pol ? ncsMul(pol, a) : ncsAdd(pol, a);
	case Tag::and:
		return pol ? ncsAdd(pol, a) : ncsMul(pol, a);

	case Tag::eqv:
	{
		auto x = at(a, 0);
		auto y = at(a, 1);

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
size_t ncsApprox(int pol, Ex* a) {
	size_t n = 0;
	if (pol >= 0) n += ncs(1, a);
	if (pol <= 0) n += ncs(0, a);
	return n;
}

// Skolem functions replace existentially quantified variables, also formulas that are renamed to avoid exponential expansion
Ex* skolem(Type* rty, Vec<Ex*>& args) {
	Vec<Ex*> v(1, gensym(ftype(rty, args.begin(), args.end())));
	// TODO: single call instead of loop?
	for (auto b: args) v.add(b);
	return ex(Tag::call, v);
}

// SORT
Vec<Ex*> defs;
size_t vars = 0;
///

// Rename formulas to avoid exponential expansion. It's tricky to do this while in the middle of doing other things, easier to be
// sure of the logic if it's done as a separate pass first.
Ex* rename(int pol, Ex* a) {
	Vec<Ex*> vars;
	freeVars(a, Vec<Ex*>(), vars);
	auto b = skolem(type(a), vars);
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
		a = ex(Tag::and, imp(b, a), imp(a, b));
		break;
	default:
		unreachable;
	}
	defs.add(quantify(a));
	return b;
}

// Maybe rename some of the arguments to an OR-over-AND (taking polarity into account), where the number of clauses generated would
// be the product of the arguments
void maybeRename(int pol, Vec<Ex*>& v) {
	// Sorting the arguments doesn't change the meaning of the formula, because AND and OR are commutative. The effect is that if
	// only some of them are to be renamed, we will leave the simple ones alone and end up renaming the complicated ones, which is
	// probably what we want.
	sort(v.begin() + 1, v.end(), [=](Ex* a, Ex* b) { return ncsApprox(pol, a) < ncsApprox(pol, b); });
	size_t n = 1;
	for (size_t i = 0; i < v.size(); ++i) {
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
Ex* maybeRename(int pol, Ex* a) {
	Vec<Ex*> v;
	// NO_SORT
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
		v.add(maybeRename(pol, at(a, 0)));
		for (size_t i = 2; i < a->n; ++i) v.add(at(a, i));
		break;

	case Tag::not:
		return ex(Tag::not, maybeRename(-pol, at(a, 0)));

	case Tag:: or:
		for (size_t i = 0; i < a->n; ++i) v.add(maybeRename(pol, at(a, i)));

		// If this formula will be used with positive polarity (including the case where it will be used both ways), we are looking
		// at OR over possible ANDs, which would produce exponential expansion at the distribution stage, so may need to rename some
		// of the arguments
		if (pol >= 0) maybeRename(pol, v);
		break;
	case Tag::and:
		for (size_t i = 0; i < a->n; ++i) v.add(maybeRename(pol, at(a, i)));

		// NOT-AND yields OR, so mirror the OR case
		if (pol <= 0) maybeRename(pol, v);
		break;

	case Tag::eqv:
	{
		auto x = maybeRename(0, at(a, 0));
		auto y = maybeRename(0, at(a, 1));
		if (ncsApprox(0, x) >= many) x = rename(0, x);
		if (ncsApprox(0, y) >= many) y = rename(0, y);
		return ex(Tag::eqv, x, y);
	}

	default:
		return a;
	}
	return ex(a->tag, v);
}

Ex* nnf(bool pol, Ex* a, Vec<pair<Ex*, Ex*>>& m);

// For-all doesn't need much work to convert. Clauses contain variables with implied for-all. The tricky part is that quantifier
// binds variables to local scope, so the same variable name used in two for-all's corresponds to two different logical variables.
// So we rename each quantified variable to a new variable of the same type.
Ex* all(int pol, Ex* a, Vec<pair<Ex*, Ex*>>& m) {
	// TODO: does it actually need to be new variables, if the parser has in any case not been allowing variable shadowing, because of types?
	auto o = m.n;
	for (size_t i = 2; i < a->n; ++i) {
		auto x = at(a, i);
		assert(x->tag == Tag::var);
		auto y = var(vars++, ((Ex*)x)->ty);
		m.add(make_pair(x, y));
	}
	a = nnf(pol, at(a, 0), m);
	m.n = o;
	return a;
}

// Each existentially quantified variable is replaced with a Skolem function whose parameters are all the surrounding universally
// quantified variables
Ex* exists(int pol, Ex* a, Vec<pair<Ex*, Ex*>>& m) {
	// Get the surrounding universally quantified variables that will be arguments to the Skolem functions
	Vec<Ex*> args;
	for (auto& xy: m)
		if (xy.second->tag == Tag::var) args.add(xy.second);

	// Make a replacement for each existentially quantified variable
	auto o = m.n;
	for (size_t i = 2; i < a->n; ++i) {
		auto x = at(a, i);
		assert(x->tag == Tag::var);
		auto y = skolem(((Ex*)x)->ty, args);
		m.add(make_pair(x, y));
	}
	a = nnf(pol, at(a, 0), m);
	m.n = o;
	return a;
}

// Negation normal form consists of several transformations that are as easy to do at the same time: Move NOTs inward to the literal
// layer, flipping things around on the way, while simultaneously resolving quantifiers
Ex* nnf(bool pol, Ex* a, Vec<pair<Ex*, Ex*>>& m) {
	Vec<Ex*> v;
	auto tag = a->tag;
	// NO_SORT
	switch (tag) {
	case Tag::false1:
		// Boolean constants and operators can be inverted by downward-sinking NOTs
		return bools + !pol;
	case Tag::true1:
		return bools + pol;

	case Tag::not:
		return nnf(!pol, at(a, 0), m);

	case Tag:: or:
		if (!pol) tag = Tag::and;
		for (size_t i = 0; i < a->n; ++i) v.add(nnf(pol, at(a, i), m));
		return ex(tag, v);
	case Tag::and:
		if (!pol) tag = Tag:: or ;
		for (size_t i = 0; i < a->n; ++i) v.add(nnf(pol, at(a, i), m));
		return ex(tag, v);

	case Tag::var:
	{
		// Variables are mapped to new variables or Skolem functions
		// TODO: does assert need to work like that?
		auto r = get(a, a, m);
		assert(r);
		return a;
	}

	case Tag::all:
		return pol ? all(pol, a, m) : exists(pol, a, m);
	case Tag::exists:
		return pol ? exists(pol, a, m) : all(pol, a, m);

	case Tag::eqv:
	{
		// Equivalence is the most difficult operator to deal with
		auto x = at(a, 0);
		auto y = at(a, 1);
		auto x0 = nnf(0, x, m);
		auto x1 = nnf(1, x, m);
		auto y0 = nnf(0, y, m);
		auto y1 = nnf(1, y, m);
		return pol ? ex(Tag::and, ex(Tag:: or, x0, y1), ex(Tag:: or, x1, y0))
				   : ex(Tag::and, ex(Tag:: or, x0, y0), ex(Tag:: or, x1, y1));
	}
	}
	for (size_t i = 0; i < a->n; ++i) v.add(nnf(1, at(a, i), m));
	a = ex(tag, v);
	return pol ? a : ex(Tag::not, a);
}

// Distribute OR down into AND, completing the layering of the operators for CNF. This is the second place where exponential
// expansion would occur, had selected formulas not already been renamed.
Ex* distribute(Ex* a) {
	Vec<Ex*> r;
	switch (a->tag) {
	case Tag:: or:
	{
		// Arguments can be taken without loss of generality as ANDs
		vector<vector<Ex*>> ands;
		for (size_t i = 0; i < a->n; ++i) {
			// Recur
			auto b = distribute(at(a, i));

			// And make a flat layer of ANDs
			vector<Ex*> v;
			flatten(Tag::and, b, v);
			ands.push_back(v);
		}

		// OR distributes over AND by Cartesian product
		// TODO: can this be done by reference?
		for (auto v: cartProduct(ands)) r.add(ex(Tag:: or, v));
		break;
	}
	case Tag::and:
		for (size_t i = 0; i < a->n; ++i) r.add(distribute(at(a, i)));
		break;
	default:
		return a;
	}
	return ex(Tag::and, r);
}

// Convert a suitably rearranged term into actual clauses
void literalsTerm(Ex* a) {
	switch (a->tag) {
	case Tag:: or:
		for (size_t i = 0; i < a->n; ++i) literalsTerm(at(a, i));
		return;
	case Tag::all:
	case Tag::and:
	case Tag::eqv:
	case Tag::exists:
		unreachable;
	case Tag::not:
		neg.add(at(a, 0));
		return;
	}
	pos.add(a);
}

bool hasNum(Ex* a) {
	if (isNum(type(a))) return 1;
	for (size_t i = 0; i < a->n; ++i)
		if (hasNum(at(a, i))) return 1;
	return 0;
}

bool hasNum(const Vec<Ex*>& v) {
	for (auto a: v)
		if (hasNum(a)) return 1;
	return 0;
}

void clausesTerm(Ex* a) {
	if (a->tag == Tag::and) {
		for (size_t i = 0; i < a->n; ++i) clausesTerm(at(a, i));
		return;
	}
	assert(!neg.n);
	assert(!pos.n);

	literalsTerm(a);

	// First-order logic is not complete on arithmetic. The conservative approach to this is that if any clause contains terms of
	// numeric type, we mark the proof search incomplete, so that failure to derive a contradiction, means the result is
	// inconclusive rather than satisfiable.
	if (result == 1 && (hasNum(neg) || hasNum(pos))) result = -1;

	clause();
}
} // namespace

void cnf(Ex* a) {
	// First run the input formula through the full process: Rename subformulas where necessary to avoid exponential expansion, then
	// convert to negation normal form, distribute OR into AND, and convert to clauses
	a = maybeRename(1, a);
	vars = 0;
	a = nnf(1, a, Vec<pair<Ex*, Ex*>>());
	a = distribute(a);
	clausesTerm(a);

	// Then convert all the definitions created by the renaming process. That process works by bottom-up recursion, which means each
	// renamed subformula is simple, so there is no need to put the definitions through the renaming process again; they just need
	// to go through the rest of the conversion steps.
	for (auto a: defs) {
		auto from = a;
		vars = 0;
		a = nnf(1, a, Vec<pair<Ex*, Ex*>>());
		a = distribute(a);
		clausesTerm(a);
	}
}
