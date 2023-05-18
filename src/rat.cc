#include "main.h"

namespace {
bool eq(Tag tag, mpq_t v, size_t n, Rat* b) {
	return tag == b->tag && mpq_equal(v, b->v);
}
bool eq(Rat* a, Rat* b) {
	return a->tag == b->tag && mpq_equal(a->v, b->v);
}

size_t hash(Tag tag, mpq_t v, size_t n) {
	return hashCombine(mpz_get_ui(mpq_numref(v)), mpz_get_ui(mpq_denref(v)));
}
size_t hash(Rat* a) {
	return hash(a->tag, ((Rat*)a)->v, 0);
}

void clear(mpq_t v) {
	mpq_clear(v);
}

Rat* make(Tag tag, mpq_t v, size_t n) {
	return new (ialloc(sizeof(Rat))) Rat(tag, v);
}

Set<Tag, mpq_t, Rat> rats;
} // namespace

Rat* rat(Tag tag, mpq_t v) {
	return rats.intern(tag, v, 0);
}
