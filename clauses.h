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

struct Formula: IFormula {
	char* file;
	char* name;
	Term* tm;
};

struct Clause: IFormula {
	uint32_t neg, n;
	Term* atoms[];
};

// Passive clauses are stored in a priority queue with smaller clauses first
size_t cost(Clause* c);

struct CompareClauses {
	bool operator()(Clause* c, Clause* d) {
		return cost(c) > cost(d);
	}
};

extern std::priority_queue<Clause*, vec<Clause*>, CompareClauses> passive;
