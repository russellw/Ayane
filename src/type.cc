#include "main.h"

LeafType tbool(Kind::boolean);
LeafType tindividual(Kind::individual);
LeafType tinteger(Kind::integer);
LeafType trat(Kind::rat);
LeafType treal(Kind::real);

bool isNum(Type* ty) {
	switch (ty->kind) {
	case Kind::integer:
	case Kind::rat:
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
		auto p = malloc(offsetof(CompType, v) + n * sizeof *v);
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

Type* compType(const Vec<Type*>& v) {
	return compType(v.data, v.n);
}

Type* compType(Type* rty, Expr** first, Expr** last) {
	if (first == last) return rty;
	Vec<Type*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return compType(v);
}

#ifdef DBG
void print(Kind kind) {
	static const char* kindNames[] = {
#define _(a) #a,
#include "kinds.h"
	};
	print(kindNames[(int)kind]);
}

void print(Type* ty) {
	switch (ty->kind) {
	case Kind::fn:
		print(at(ty, 0));
		putchar('(');
		for (size_t i = 1; i < ty->n; ++i) {
			if (i > 1) print(", ");
			print(at(ty, i));
		}
		putchar(')');
		return;
	case Kind::name:
		print(((TypeName*)ty)->s);
		return;
	}
	print(ty->kind);
}
#endif
