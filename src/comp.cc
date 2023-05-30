#include "main.h"

static Expr* comp(Tag tag, Expr** v, size_t n) {
	// TODO: check generated code
	auto a = new (balloc(sizeof(Comp) + n * sizeof(void*))) Comp(tag, n);
	memcpy(a->v, v, n * sizeof(void*));
	return a;
}

Expr* comp(Tag tag, Expr* a) {
	return comp(tag, &a, 1);
}

Expr* comp(Tag tag, Expr* a, Expr* b) {
	Expr* v[2];
	v[0] = a;
	v[1] = b;
	return comp(tag, v, 2);
}

Expr* comp(Tag tag, Vec<Expr*>& v) {
	return comp(tag, v.data, v.n);
}

namespace {
bool eq(Tag tag, Expr** a, size_t n, Comp* b) {
	assert(a);
	return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof(void*)) == 0;
}
bool eq(Comp* a, Comp* b) {
	return eq(a->tag, a->v, a->n, b);
}

size_t hash(Tag tag, Expr** a, size_t n) {
	// TODO: hashCombine?
	return fnv(a, n * sizeof(void*));
}
size_t hash(Comp* a) {
	return hash(a->tag, a->v, a->n);
}

void clear(Expr** a) {
}

Comp* make(Tag tag, Expr** v, size_t n) {
	auto a = new (ialloc(sizeof(Comp) + n * sizeof(void*))) Comp(tag, n);
	memcpy(a->v, v, n * sizeof(void*));
	return a;
}

#include "set.h"
Set<Tag, Expr**, Comp, 0x1000> comps;
} // namespace

Expr* compi(Tag tag, Vec<Expr*>& v) {
	return comps.intern(tag, v.data, v.n);
}
