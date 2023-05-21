// First-order variables must be of non-Boolean leaf types
struct Var: Expr {
	size_t i;
	LeafType* ty;

	Var(size_t i, LeafType* ty): Expr(Tag::var), i(i), ty(ty) {
	}
};

Var* var(size_t i, LeafType* ty);
