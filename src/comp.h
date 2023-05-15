// Composite expressions. Built-in operators like equality and addition are represented by tags. The most important variety of
// composite expression is the function call, represented by Tag::call, with the function as v[0], so in that case the arguments
// start at 1.
struct Comp: Expr {
	Expr* v[];

	Comp(Tag tag, size_t n): Expr(tag) {
		this->n = n;
	}
};

inline Expr** begin(Expr* a) {
	return ((Comp*)a)->v;
}
inline Expr** end(Expr* a) {
	return ((Comp*)a)->v + a->n;
}

inline Expr* at(Expr* a, size_t i) {
	assert(i < a->n);
	return ((Comp*)a)->v[i];
}

// TODO: test using a bump allocator
Expr* comp(Tag tag, Expr* a);
Expr* comp(Tag tag, Expr* a, Expr* b);
Expr* comp(Tag tag, Vec<Expr*>& v);
Expr* comp(Tag tag, vector<Expr*>& v);

Expr* compc(Tag tag, Vec<Expr*>& v);
