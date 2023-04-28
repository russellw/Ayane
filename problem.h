struct Problem {
	// First-order logic problems are sets of formulas; in addition to the formulas themselves, we also need to track where they
	// came from, for use when printing proofs. While the formulas themselves are handled as local variables so that the interfaces
	// can be reused by internal reasoning code, the information about source files might as well be kept in a global variable. If
	// the same formula occurs multiple times under different names, there is no need to retain more than one of them. (Unless one
	// is a conjecture, which is a separate thing; see below.)

	// In the DIMACS and TPTP formats, clauses can be provided directly, but this is not as simple as it sounds, because there is
	// still the distinction between the not necessarily simplified input version and the simplified version required by proof
	// procedures, and since this kind of input is rarely used nowadays, there is no good reason to provide a special code path for
	// it. So input clauses are just treated as formulas.

	// The initial formulas are the ones the problem contained prior to CNF conversion. They exclude the conjecture, but include the
	// negated conjecture (if any).
	map<term, uint32_t> initialFormulas;

	// What happens if a formula shows up twice in the input? This is redundant; one occurrence will be kept, and the other
	// discarded.

	// What if one shows up as both an axiom and a negated conjecture? This is the same thing; one occurrence will be kept, the
	// other discarded.

	// What if one shows up as both an axiom and a conjecture? This is not redundant; it makes the problem an easy theorem. The
	// axiom is kept, along with the negated conjecture.

	// Most of the logic doesn't care whether the problem contained a conjecture, but it needs to be remembered for e.g. the
	// specifics of SZS output.
	bool hasConjecture = 0;

	// Add formulas at input time. Some languages don't give names to formulas, but it should always be possible to remember what
	// file they came from.
	void axiom(term a, const char* file, const char* name = 0);
	void conjecture(term a, const char* file, const char* name = 0);

	// In the event of the problem being successfully solved, we may end up printing a proof. This involves converting the
	// hypergraph of what was derived from what, to a list of formulas that made some contribution to the result, in order of
	// derivation, each to be printed exactly once.
private:
	map<term, uint32_t> visitedFormulas;
	map<clause, uint32_t> visitedcs;

	size_t walk(term a);
	size_t walk(const ProofCnf& proofCnf, const Proof& proof, Clause* c);

public:
	vec<uint32_t> proofv;
	void setProof(const ProofCnf& proofCnf, const Proof& proof);
	// TODO: destructor
};
