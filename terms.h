enum {
#define _(x) x,
#include "tags.h"
	ntags
};

// TODO: rename to expr?
// TODO: refactor source file organization?
struct Term {
	int tag;
	uint32_t n;
};

// Atoms
struct Atom: Term {
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
Atom* term(mpz_t val);
Atom* term(mpq_t val);

// Wrapping a symbol in a term is a common operation. Specifying the type at the same time is less so, but still common enough for
// this function to be useful.
Atom* term(string* s, type ty);

inline Atom* tbool(bool b) {
	return atoms + (b ? True : False);
}

Atom* var(size_t i, type ty);

Atom* gensym(type ty);
Atom* distinctObj(string* s);

// Compound terms contain other terms, but maintain value semantics. Like atoms, they are interned.
struct Compound {
	Term* v[];
};

// TODO: test using a bump allocator
Term* term(int tag);
Term* term(int tag, Term* a);
Term* term(int tag, Term* a, Term* b);
Term* term(Term* a, Term* b);
Term* term(Term* a, Term* b, Term* c);
Term* term(Term* a, Term* b, Term* c, Term* d);
Term* term(Term* a, Term* b, Term* c, Term* d, Term* e);
Term* term(const vec<Term*>& v);
Term* term(const std::vector<Term*>& v);
type getType(Term* a);

inline Term* at(Term* a, size_t i) {
	assert(i < a->n);
	return ((Compound*)a)->v[i];
}

type ftype(type rty, const Term* first, const Term* last);
type ftype(type rty, const vec<Term*>& args);
void check(Term* a, type ty);

void print(Term* a);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using termx = pair<Term*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
// TODO: this is not just a pair of arbitrary terms; it has special rules. Should it be a separate type?
using Eqn = pair<Term*, Term*>;
