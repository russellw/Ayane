#include "main.h"

struct Cmp {
	// TODO: if this ends up also needing allocator, rename to something like compElement?
	static bool eq(Kind kind, Type** a, size_t n, CompType* b) {
		return n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
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

	static CompType* make(Kind kind, Type** v, size_t n) {
		auto p = malloc(sizeof(CompType) + n * sizeof *v);
		auto a = new (p) CompType(kind, n);
		memcpy(a->v, v, n * sizeof *v);
		return a;
	}
};

static void clear(Type** a) {
}

static Set<Kind, Type**, CompType, Cmp> compTypes;

Type* compType(Type** v, size_t n) {
	assert(n);
	if (n == 1) return *v;
	return compTypes.intern(Kind::fn, v, n);
}

Type* compType(Type* a, Type* b) {
	static Type* v[2];
	v[0] = a;
	v[1] = b;
	return compType(v, 2);
}

Type* compType(Vec<Type*>& v) {
	return compType(v.data, v.n);
}

Type* compType(Type* rty, Expr** first, Expr** last) {
	if (first == last) return rty;
	Vec<Type*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return compType(v);
}
