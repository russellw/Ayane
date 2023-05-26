// First-order variables must be of non-Boolean leaf types
struct Var: Expr {
	LeafType* ty;
	size_t i;

	Var(LeafType* ty, size_t i): Expr(Tag::var), ty(ty), i(i) {
	}
};

Var* var(LeafType* ty, size_t i);
