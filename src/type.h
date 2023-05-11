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

// In typed first-order logic, we can name a type, and then work with it without needing to know what it is made of
struct TypeName: LeafType {
	char* s;
};

TypeName* typeName(Str* s);

// At the moment, the only composite types understood by the system, are functions specified by parameters and return type. These
// are represented with the return type as v[0]. This does not match the convention the system otherwise tries to follow from logic
// notation where the type is written after an expression, but does match the representation of function calls, that starts with the
// function.
struct CompType: Type {
	Type* v[];

	CompType(Kind kind, size_t n): Type(kind) {
		this->n = n;
	}
};

inline Type* at(Type* a, size_t i) {
	assert(i < a->n);
	return ((CompType*)a)->v[i];
}

Type* compType(Type* a, Type* b);
Type* compType(const Vec<Type*>& v);
// TODO: rename params
Type* compType(Type* rty, Expr** first, Expr** last);

#ifdef DBG
void print(Kind kind);
void print(Type* ty);
#endif
