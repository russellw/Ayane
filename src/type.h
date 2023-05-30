enum class Kind {
#define _(a) a,
#include "kinds.h"
};

struct Type {
	Kind kind;
	uint32_t n = 0;

	Type(Kind kind): kind(kind) {
	}
};

struct LeafType: Type {
	// In first-order logic, variables can only have leaf types
	Vec<Var*> vars;

	// Matching or unification between two clauses, assumes the clauses have distinct variable names. The simplest way to ensure
	// this is to rename the variables in one clause, to an alternate set.
	Vec<Var*> alts;

	LeafType(Kind kind): Type(kind) {
	}
};

extern LeafType tbool;
extern LeafType tindividual;
extern LeafType tinteger;
extern LeafType trat;
extern LeafType treal;

bool isNum(Type* t);

// In typed first-order logic, we can name a type, and then work with it without needing to know what it is made of
struct OpaqueType: LeafType {
	char* s;

	OpaqueType(char* s): LeafType(Kind::opaque), s(s) {
	}
};

#ifdef DBG
void dbgPrint(Kind kind);
void dbgPrint(Type* t);
#endif
