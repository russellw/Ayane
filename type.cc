#include "main.h"

// TODO: rename to boolean?
Type tbool = {True};
Type tindividual = {Individual};
Type tinteger = {Integer};
Type trational = {Rational};
Type treal = {Real};

// Composite types
struct CompCmp {
	// TODO: if this ends up also needing allocator, rename to something like compElement?
	static bool eq(Kind tag, Type** a, size_t n, Type* b) {
		return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
	}
	static bool eq(Type* a, Type* b) {
		return eq(a->tag, a->v, a->n, b);
	}

	static size_t hash(Kind tag, Type** a, size_t n) {
		// TODO: hashCombine?
		return fnv(a, n * sizeof *a);
	}
	static size_t hash(Type* a) {
		return hash(a->tag, a->v, a->n);
	}
};

static void clear(Type** a) {
}

static Set<Kind, Type**, Type, CompCmp> comps;

bool isNum(Type* ty) {
	switch (ty->kind) {
	case Kind::integer:
	case Kind::rational:
	case Kind::real:
		return 1;
	}
	return 0;
}
