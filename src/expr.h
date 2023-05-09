enum class Tag {
#define _(a) a,
#include "tags.h"
};

// TODO: refactor source file organization
// TODO: rename
struct Expr {
	Tag tag;
	uint32_t n;
	union {
		// TODO: check size of v
		Expr* v[9];
		struct {
			// TODO: can this be omitted for numbers?
			Type* ty;
			union {
				char* s;
				size_t idx;
				mpz_t mpz;
				mpq_t mpq;
			};
		};
	};
};

extern Expr bools[2];

Expr* ex(mpz_t val);
Expr* ex(mpq_t val);

Expr* var(size_t i, Type* ty);

Expr* gensym(Type* ty);

// TODO: test using a bump allocator
Expr* ex(Tag tag, Expr* a);
Expr* ex(Tag tag, Expr* a, Expr* b);
Expr* ex(Tag tag, const Vec<Expr*>& v);
Expr* ex(Tag tag, const vector<Expr*>& v);
Type* type(Expr* a);

inline Expr* at(Expr* a, size_t i) {
	assert(i < a->n);
	return a->v[i];
}

int cmp(Expr* a, Expr* b);

Type* ftype(Type* rty, Expr** first, Expr** last);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using Ex2 = pair<Expr*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
struct Eqn: pair<Expr*, Expr*> {
	Eqn(Expr* a);
};
