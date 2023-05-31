// Arbitrary precision rationals. These also represent real numbers (well, the rational subset of the reals), distinguished (for the
// sake of the type system) by tag
struct Rat: Expr {
	mpq_t v;

	Rat(Tag tag, mpq_t v): Expr(tag) { memcpy(this->v, v, sizeof this->v); }
};

Rat* rat(Tag tag, mpq_t v);
