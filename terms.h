enum {
#define _(x) x,
#include "tags.h"
	end
};

struct term {
	int tag;
	uint32_t n;
};

// Atoms
struct atom: term {
	type ty;
	union {
		const char* s;
		size_t idx;
		mpz_t mpz;
		mpq_t mpq;
	};
};

extern atom atoms[];

term* mk(mpz_t val);
term* mk(mpq_t val);

// Wrapping a symbol in a term is a common operation. Specifying the type at the same time is less so, but still common enough for
// this function to be useful.
term* mk(string* s, type ty);

inline term* mkbool(bool b) {
	return atoms + (b ? True : False);
}

term* var(size_t i, type ty);

term* gensym(type ty);
term* distinctObj(string* s);

// Compound terms contain other terms, but maintain value semantics. Like atoms, they are interned.
struct compound {
	term* v[];
};

term* mk(int tag, term* a);
term* mk(term* a, term* b);
term* mk(term* a, term* b, term* c);
term* mk(term* a, term* b, term* c, term* d);
term* mk(term* a, term* b, term* c, term* d, term* e);
term* mk(const vec<term>& v);
type getType(term* a);

inline term* at(term* a, size_t i) {
	assert(i < a->n);
	return ((compound*)a)->v[i];
}

type ftype(type rty, const term* first, const term* last);
type ftype(type rty, const set<term>& args);

inline size_t hash(const term& a) {
	return fnv(&a, sizeof a);
}

void print(term* a);
