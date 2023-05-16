// At the moment, the only composite types understood by the system, are functions specified by parameters and return type. These
// are represented with the return type as v[0]. This does not match the convention the system otherwise tries to follow from logic
// notation where the type is written after an expression, but does match the representation of function calls, that starts with the
// function.
struct CompType: Type {
	Type* v[];

	CompType(Kind kind, size_t n): Type(kind) {
		this->n = n;
	}
};

inline Type* at(Type* a, size_t i) {
	assert(i < a->n);
	return ((CompType*)a)->v[i];
}

Type* compType(Type* a, Type* b);
Type* compType(Vec<Type*>& v);
