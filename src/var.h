// First-order variables must be of non-Boolean leaf types
struct Var: Expr {
	LeafType* t;
	size_t i;

	Var(LeafType* t, size_t i): Expr(Tag::var), t(t), i(i) {
	}
};

Var* var(LeafType* t, size_t i);
