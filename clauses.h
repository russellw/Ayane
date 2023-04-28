// Names of the inference rules used by the program, for use in tracking inferences and printing proofs
enum {
#define _(x) r_##x,
#include "rules.h"
};

extern const char* ruleNames[];

struct IFormula {
	IFormula* from[2];
};

struct Formula: IFormula {
	char* file;
	char* name;
	Term* tm;
};

struct Clause: IFormula {};
