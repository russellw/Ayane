#include "main.h"

struct RatElement {
	// TODO: can we now remove the Cmp class?
	static bool eq(Tag tag, mpq_t v, size_t n, Rat* b) {
		return tag == b->tag && mpq_equal(v, b->v);
	}
	static bool eq(Rat* a, Rat* b) {
		return a->tag == b->tag && mpq_equal(a->v, b->v);
	}

	static size_t hash(Tag tag, mpq_t v, size_t n) {
		return hashCombine(mpz_get_ui(mpq_numref(v)), mpz_get_ui(mpq_denref(v)));
	}
	static size_t hash(Rat* a) {
		return hash(a->tag, ((Rat*)a)->v, 0);
	}

	static void clear(mpq_t v) {
		mpq_clear(v);
	}

	static Rat* make(Tag tag, mpq_t v, size_t n) {
		return new (ialloc(sizeof(Rat))) Rat(tag, v);
	}
};

static Set<Tag, mpq_t, Rat, RatElement> rats;

Rat* rat(Tag tag, mpq_t v) {
	return rats.intern(tag, v, 0);
}
