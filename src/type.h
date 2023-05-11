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

struct TypeName: LeafType {
	char* s;
};

struct CompType: Type {
	Type* v[];
};

extern LeafType tbool;
extern LeafType tindividual;
extern LeafType tinteger;
extern LeafType trational;
extern LeafType treal;

inline Type* at(Type* a, size_t i) {
	assert(i < a->n);
	return ((CompType*)a)->v[i];
}

Type* compType(Type* a, Type* b);
Type* compType(const Vec<Type*>& v);
bool isNum(Type* ty);
// TODO: rename params
Type* compType(Type* rty, Expr** first, Expr** last);
TypeName* typeName(Str* s);
