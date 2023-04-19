#include "main.h"

template <class T> T* alloc(uint32_t& o) {
	o = heap->alloc(sizeof(T));
	return (T*)heap->ptr(o);
}

void Problem::axiom(term a, const char* file, const char* name) {
	// Check where to put a formula object corresponding to this term.
	auto& o = initialFormulas.gadd(a);

	// If we have already recorded an identical formula, return.
	if (o) return;

	// Placing an object at an address aligned only to 4 bytes, but some fields of the object are pointers, which are typically 8
	// bytes. Depending on CPU, this may incur a slight slowdown accessing those fields. That's fine, because they are rarely
	// accessed. If we ever need to run on a CPU where it's more than a slight slowdown, it might be necessary to look for another
	// solution.
	auto f = alloc<InputFormula>(o);
	new (f) InputFormula(FormulaClass::Axiom, a, file, name);
}

void Problem::conjecture(term a, const char* file, const char* name) {
	// If multiple conjectures occur in a problem, there are two possible interpretations (conjunction or disjunction), and no
	// consensus on which is correct, so rather than risk silently giving a wrong answer, reject the problem as ambiguous and
	// require it to be restated with the conjectures folded into one, using explicit conjunction or disjunction.
	if (hasConjecture) err("Multiple conjectures not supported");
	hasConjecture = 1;

	// The formula actually added to the set whose satisfiability is to be tested, is the negated conjecture.
	auto b = term(tag::Not, a);

	// Check where to put a formula object corresponding to this term.
	auto& o = initialFormulas.gadd(b);

	// If we have already recorded an identical formula, return.
	if (o) return;

	// Make a formula object for the conjecture. This will not be added to the set of clauses, only used as the source of the
	// negated conjecture.
	uint32_t conp;
	auto conf = alloc<InputFormula>(conp);
	new (conf) InputFormula(FormulaClass::Conjecture, a, file, name);

	// Make and place the formula for the negated conjecture.
	auto f = alloc<NegatedFormula>(o);
	new (f) NegatedFormula(b, conp);
}

size_t Problem::walk(term a) {
	auto& o = visitedFormulas.gadd(a);
	if (o) return o;

	// Check this term against the initial formulas.
	uint32_t i;
	if (initialFormulas.get(a, i)) {
		// This is one of the initial formulas, so already has a formula object; we don't need to make one.
		auto f = (AbstractFormula*)heap->ptr(i);

		// But if it is the negated conjecture, we do need to add the conjecture before it, to the list of formulas to print out in
		// the proof.
		if (f->Class == FormulaClass::Negation) proofv.push_back(((NegatedFormula*)f)->from);
	} else {
		// Not one of the initial formulas, so it must be a definition introduced during CNF conversion. Make a formula object for
		// it. (Some definitions may not be involved in the proof. By waiting to make formula objects until they are definitely
		// needed, we can entirely avoid doing it for those definitions.)
		auto f = alloc<Formula>(i);
		new (f) Formula(FormulaClass::Definition, a);
	}

	// Remember that we already visited this formula, in case it is referred to multiple times in the proof.
	o = i;

	// Add it to the list of formulas to be printed out.
	proofv.push_back(i);

	// And return a reference to the object.
	return i;
}

size_t Problem::walk(const ProofCnf& proofCnf, const Proof& proof, const clause& c) {
	auto& o = visitedcs.gadd(c);
	if (o) return o;

	// Allocate an object representing this clause, and also remember that we already visited it.
	auto f = alloc<Clause>(o);

	// Need to be careful here: We have a pointer to the allocated object, in the 32-bit offset format we will need to return, and
	// it would be easy to just return it at the end of the function. But it is actually currently being held by reference, and may
	// become invalid if recursive calls result in more clauses being added to the map, triggering a reallocation, so make a copy of
	// it by value.
	auto r = o;

	// Now we need to check where it came from.
	rule rl;
	term a;
	size_t from;
	size_t from1 = 0;
	if (proofCnf.get(c, a)) {
		// The clause was CNF-converted from a formula.
		rl = rule::cnf;
		from = walk(a);
	} else {
		// Otherwise, it must have been inferred from other clauses.
		auto& derivation = proof.at(c);
		rl = derivation.first;
		from = walk(proofCnf, proof, derivation.second[0]);
		if (derivation.second.size() == 2) from1 = walk(proofCnf, proof, derivation.second[1]);
	}

	// Fill in the allocated object.
	new (f) Clause(c, rl, from, from1);

	// Add it to the list of clauses to be printed out, in order after its sources.
	proofv.push_back(r);

	// And return a reference to the object.
	return r;
}

void Problem::setProof(const ProofCnf& proofCnf, const Proof& proof) {
	walk(proofCnf, proof, falsec);
}
