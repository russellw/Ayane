#include "main.h"

namespace {
bool eq(Kind kind, Type** a, size_t n, CompType* b) {
	return n == b->n && memcmp(a, b->v, n * sizeof(void*)) == 0;
}
bool eq(CompType* a, CompType* b) {
	return eq(a->kind, a->v, a->n, b);
}

size_t hash(Kind kind, Type** a, size_t n) {
	// TODO: hashCombine?
	return fnv(a, n * sizeof(void*));
}
size_t hash(CompType* a) {
	return hash(a->kind, a->v, a->n);
}

void clear(Type** a) {
}

CompType* make(Kind kind, Type** v, size_t n) {
	auto a = new (ialloc(sizeof(CompType) + n * sizeof(void*))) CompType(kind, n);
	memcpy(a->v, v, n * sizeof(void*));
	return a;
}

Set<Kind, Type**, CompType> compTypes;
} // namespace

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
