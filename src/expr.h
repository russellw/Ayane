enum class Tag {
#define _(a) a,
#include "tags.h"
};

// TODO: refactor source file organization
struct Expr {
	Tag tag;
	uint32_t n = 0;

	Expr(Tag tag): tag(tag) {
	}
};

struct Fn: Expr {
	char* name;
	Type* ty;

	Fn(char* name, Type* ty): Expr(Tag::fn), name(name), ty(ty) {
	}
};

struct Var: Expr {
	Type* ty;

	Var(Type* ty): Expr(Tag::var), ty(ty) {
	}
};

struct Integer: Expr {
	mpz_t val;

	Integer(mpz_t val): Expr(Tag::integer) {
		// TODO: check  sizeof
		memcpy(this->val, val, sizeof this->val);
	}
};

struct Rational: Expr {
	mpq_t val;

	Rational(Tag tag, mpq_t val): Expr(tag) {
		memcpy(this->val, val, sizeof this->val);
	}
};

struct Comp: Expr {
	Expr* v[];

	Comp(Tag tag, size_t n): Expr(tag) {
		this->n = n;
	}
};

extern Expr bools[2];

Expr* expr(mpz_t val);
Expr* expr(mpq_t val);

Expr* var(size_t i, Type* ty);

// TODO: test using a bump allocator
Expr* expr(Tag tag, Expr* a);
Expr* expr(Tag tag, Expr* a, Expr* b);
Expr* expr(Tag tag, const Vec<Expr*>& v);
Expr* expr(Tag tag, const vector<Expr*>& v);
Type* type(Expr* a);

inline Expr* at(Expr* a, size_t i) {
	assert(i < a->n);
	return ((Comp*)a)->v[i];
}

int cmp(Expr* a, Expr* b);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using ExprSubscript = pair<Expr*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
struct Eqn: pair<Expr*, Expr*> {
	Eqn(Expr* a);
};