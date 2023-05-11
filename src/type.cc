#include "main.h"

// TODO: rename to boolean?
LeafType tbool(Kind::boolean);
LeafType tindividual(Kind::individual);
LeafType tinteger(Kind::integer);
LeafType trational(Kind::rational);
LeafType treal(Kind::real);

bool isNum(Type* ty) {
	switch (ty->kind) {
	case Kind::integer:
	case Kind::rational:
	case Kind::real:
		return 1;
	}
	return 0;
}

TypeName* typeName(Str* s) {
	if (s->ty) return s->ty;
	// TODO: Worth using a constructor?
	auto ty = (TypeName*)malloc(sizeof(TypeName));
	ty->kind = Kind::name;
	ty->n = 0;
	ty->s = s->v;
	return s->ty = ty;
}

// Composite types
struct Cmp {
	// TODO: if this ends up also needing allocator, rename to something like compElement?
	static bool eq(Kind kind, Type** a, size_t n, CompType* b) {
		return kind == b->kind && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
	}
	static bool eq(CompType* a, CompType* b) {
		return eq(a->kind, a->v, a->n, b);
	}

	static size_t hash(Kind kind, Type** a, size_t n) {
		// TODO: hashCombine?
		return fnv(a, n * sizeof *a);
	}
	static size_t hash(CompType* a) {
		return hash(a->kind, a->v, a->n);
	}
};

static void clear(Type** a) {
}

static Set<Kind, Type**, CompType, Cmp> compTypes;

Type* compType(Type* a, Type* b) {
	return compType(a, &b, &b + 1);
}

Type* compType(Type* rty, Expr** first, Expr** last) {
	if (first == last) return rty;
	Vec<Type*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return compType(v);
}
