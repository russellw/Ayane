#include "main.h"

// Numbers need to be interned, to preserve the property that equal expressions have pointer equality
struct IntElement {
	static bool eq(Tag tag, mpz_t v, size_t n, Int* b) {
		return mpz_cmp(v, b->v) == 0;
	}
	static bool eq(Int* a, Int* b) {
		return mpz_cmp(a->v, b->v) == 0;
	}

	static size_t hash(Tag tag, mpz_t v, size_t n) {
		return mpz_get_ui(v);
	}
	static size_t hash(Int* a) {
		return hash(Tag::integer, ((Int*)a)->v, 0);
	}

	static void clear(mpz_t v) {
		mpz_clear(v);
	}

	static Int* make(Tag tag, mpz_t v, size_t n) {
		return new (ialloc(sizeof(Int))) Int(v);
	}
};

static Set<Tag, mpz_t, Int, IntElement> integers;

Int* integer(mpz_t v) {
	return integers.intern(Tag::integer, v, 0);
}
