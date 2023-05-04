// Names of the inference rules used by the program, for use in tracking inferences and printing proofs
enum {
#define _(x) r_##x,
#include "rules.h"
};

extern const char* ruleNames[];

struct IFormula {
	int rule;
	IFormula* from[2];
};

// In the DIMACS and TPTP formats, clauses can be provided directly, but this is not as straightforward as it sounds, because there
// is still the distinction between the not necessarily simplified input version and the simplified version required by proof
// procedures, and since this kind of input is rarely used nowadays, there is no good reason to provide a special code path for it.
// So input clauses are just treated as formulas.
struct Formula: IFormula {
	const char* file;
	const char* name;
	Ex* tm;

	Formula(const char* file, const char* name, Ex* tm): file(file), name(name), tm(tm) {
		rule = r_file;
		*from = 0;
	}
};

// Most of the logic doesn't care whether the problem contained a conjecture, but it needs to be remembered for e.g. the specifics
// of SZS output.
extern Formula* conjecture;

// Are clauses sets of literals, or bags? It would seem logical to represent them as sets, and some algorithms prefer it that way,
// but unfortunately there are important algorithms that will break unless they are represented as bags, such as the superposition
// calculus:
// https://stackoverflow.com/questions/29164610/why-are-clauses-multisets
// So we represent them as bags (or lists, ignoring the order) and let the algorithms that prefer sets, discard duplicate literals
struct Clause: IFormula {
	uint32_t neg, n;
	Ex* atoms[];
};

range neg() const {
	return range(0, neg_n);
}
range pos() const {
	return range(neg_n, size());
}

inline Ex* at(Clause* c, size_t i) {
	assert(i < c->n);
	return c->atoms[i];
}

// Passive clauses are stored in a priority queue with smaller clauses first
size_t cost(Clause* c);

struct CompareClauses {
	bool operator()(Clause* c, Clause* d) {
		return cost(c) > cost(d);
	}
};

extern std::priority_queue<Clause*, vec<Clause*>, CompareClauses> passive;
void clause(vec<Ex*>& neg, vec<Ex*>& pos, int rule, IFormula* from, IFormula* from1 = 0);
