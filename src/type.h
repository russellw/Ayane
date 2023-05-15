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

	LeafType(Kind kind): Type(kind) {
	}
};

extern LeafType tbool;
extern LeafType tindividual;
extern LeafType tinteger;
extern LeafType trat;
extern LeafType treal;

bool isNum(Type* ty);

#ifdef DBG
void print(Kind kind);
void print(Type* ty);
#endif
