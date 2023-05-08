enum {
	// TODO: True needs to be early in the list, but does False need any particular order?
	False,
	True,

	// TODO: use everywhere or nowhere?
	// SORT
	Add,
	All,
	And,
	Call,
	Ceil,
	Div,
	DivE,
	DivF,
	DivT,
	Eq,
	Eqv,
	Exists,
	Floor,
	Fn,
	Individual,
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

struct Type {
	int tag;
	uint32_t n;
};

struct TypeName: Type {
	char* s;
};

struct CompType: Type {
	Type* v[];
};

// First-order logic types
extern Type tbool;
extern Type tindividual;

// Number types
extern Type tinteger;
extern Type trational;
extern Type treal;

inline Type* at(Type* a, size_t i) {
	assert(i < a->n);
	return ((CompType*)a)->v[i];
}

Type* type(Type* a, Type* b);
Type* type(const vec<Type*>& v);
