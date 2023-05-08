// TODO: refactor source file organization
struct Ex {
	int tag;
	uint32_t n;
	union {
		// TODO: check size of v
		Ex* v[9];
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

extern Ex bools[2];

Ex* ex(mpz_t val);
Ex* ex(mpq_t val);

Ex* var(size_t i, Type* ty);

Ex* gensym(Type* ty);

// TODO: test using a bump allocator
Ex* ex(int tag, Ex* a);
Ex* ex(int tag, Ex* a, Ex* b);
Ex* ex(int tag, const vec<Ex*>& v);
Ex* ex(int tag, const vector<Ex*>& v);
Type* type(Ex* a);

inline Ex* at(Ex* a, size_t i) {
	assert(i < a->n);
	return a->v[i];
}

int cmp(Ex* a, Ex* b);

Type* ftype(Ex* rty, Ex** first, Ex** last);

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, those are not
// defined as a separate class, so it is defined in terms of terms.
using exx = pair<Ex*, bool>;

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
struct eqn: pair<Ex*, Ex*> {
	eqn(Ex* a);
};
