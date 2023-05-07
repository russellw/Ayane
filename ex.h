enum {
	False,
	True,

	// TODO: use everywhere or nowhere?
	// SORT
	Add,
	All,
	And,
	Bool,
	Call,
	Ceil,
	DistinctObj,
	Div,
	DivE,
	DivF,
	DivT,
	Eq,
	Eqv,
	Exists,
	Floor,
	Fn,
	Integer,
	IsInteger,
	IsRational,
	Le,
	Lt,
	Mul,
	Neg,
	Not,
	Or,
	Rational,
	Real,
	RemE,
	RemF,
	RemT,
	Round,
	Sub,
	ToInteger,
	ToRational,
	ToReal,
	Trunc,
	Var,
	///
};

// TODO: refactor source file organization?
struct Ex {
	int tag;
	uint32_t n;
	union {
		// TODO: check
		Ex* v[9];
		struct {
			Ex* ty;
			union {
				char* s;
				size_t idx;
				mpz_t mpz;
				mpq_t mpq;
			};
		};
	};
};

extern Ex tbool;
extern Ex tinteger;
extern Ex trational;
extern Ex treal;

extern Ex bools[2];

Ex* ex(mpz_t val);
Ex* ex(mpq_t val);

Ex* var(size_t i, Ex* ty);

Ex* gensym(Ex* ty);

// TODO: test using a bump allocator
Ex* ex(int tag, Ex* a);
Ex* ex(int tag, Ex* a, Ex* b);
Ex* ex(int tag, const vec<Ex*>& v);
Ex* ex(int tag, const vector<Ex*>& v);
Ex* type(Ex* a);

inline Ex* at(Ex* a, size_t i) {
	assert(i < a->n);
	return a->v[i];
}

int cmp(Ex* a, Ex* b);

Ex* ftype(Ex* rty, const Ex* first, const Ex* last);
Ex* ftype(Ex* rty, const vec<Ex*>& args);
void check(Ex* a, Ex* ty);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using termx = pair<Ex*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
struct eqn: pair<Ex*, Ex*> {
	eqn(Ex* a);
};
