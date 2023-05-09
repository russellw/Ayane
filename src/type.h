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

struct TypeName: Type {
	char* s;
};

struct CompType: Type {
	Type* v[];
};

extern Type tbool;
extern Type tindividual;
extern Type tinteger;
extern Type trational;
extern Type treal;

inline Type* at(Type* a, size_t i) {
	assert(i < a->n);
	return ((CompType*)a)->v[i];
}

Type* compType(Type* a, Type* b);
Type* compType(const Vec<Type*>& v);
bool isNum(Type* ty);
Type* compType(Type* rty, Expr** first, Expr** last);
Type* typeName(Str* s);
