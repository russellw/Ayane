enum {
#define _(x) x,
#include "tags.h"
	ntags
};

// TODO: rename to Ex, Leaf, Comp?
// TODO: refactor source file organization?
struct Ex {
	int tag;
	uint32_t n;
};

// Atoms
struct Atom: Ex {
	type ty;
	union {
		const char* s;
		size_t idx;
		mpz_t mpz;
		mpq_t mpq;
	};
};

extern Atom atoms[];

// TODO: rename to 'atom'?
Atom* ex(mpz_t val);
Atom* ex(mpq_t val);

// Wrapping a symbol in a term is a common operation. Specifying the type at the same time is less so, but still common enough for
// this function to be useful.
Atom* ex(string* s, type ty);

inline Atom* tbool(bool b) {
	return atoms + (b ? True : False);
}

Atom* var(size_t i, type ty);

Atom* gensym(type ty);
Atom* distinctObj(string* s);

// Compound terms contain other terms, but maintain value semantics. Like atoms, they are interned.
struct Compound {
	Ex* v[];
};

// TODO: test using a bump allocator
Ex* ex(int tag);
Ex* ex(int tag, Ex* a);
Ex* ex(int tag, Ex* a, Ex* b);
Ex* ex(Ex* a, Ex* b);
Ex* ex(Ex* a, Ex* b, Ex* c);
Ex* ex(Ex* a, Ex* b, Ex* c, Ex* d);
Ex* ex(Ex* a, Ex* b, Ex* c, Ex* d, Ex* e);
Ex* ex(int tag, const vec<Ex*>& v);
Ex* ex(int tag, const std::vector<Ex*>& v);
type getType(Ex* a);

inline Ex* at(Ex* a, size_t i) {
	assert(i < a->n);
	return ((Compound*)a)->v[i];
}

type ftype(type rty, const Ex* first, const Ex* last);
type ftype(type rty, const vec<Ex*>& args);
void check(Ex* a, type ty);

void print(Ex* a);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using termx = pair<Ex*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
// TODO: this is not just a pair of arbitrary terms; it has special rules. Should it be a separate type?
using Eqn = pair<Ex*, Ex*>;
