// First-order variables must be of non-Boolean leaf types
struct Var: Expr {
	LeafType* ty;

	Var(LeafType* ty): Expr(Tag::var), ty(ty) {
	}
};

Var* var(size_t i, LeafType* ty);
