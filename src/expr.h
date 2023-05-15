enum class Tag {
#define _(a) a,
#include "tags.h"
};

// Expressions are the most important data structures in the system. Logic formulas are expressions of Boolean type; terms are
// expressions of other types.
struct Expr {
	Tag tag;
	uint32_t n = 0;

	Expr(Tag tag): tag(tag) {
	}
};

// The Boolean constants are primitive expressions. False and true are represented as &bools[value], i.e. bools and bools+1
// respectively.
extern Expr bools[2];

Type* type(Expr* a);

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
struct Eqn: pair<Expr*, Expr*> {
	Eqn(Expr* a);
};

#ifdef DBG
void print(Tag tag);
void print(Expr* a);
#endif
