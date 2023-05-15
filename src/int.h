struct Int: Expr {
	mpz_t v;

	Int(mpz_t v): Expr(Tag::integer) {
		// TODO: check  sizeof
		memcpy(this->v, v, sizeof this->v);
	}
};

Int* integer(mpz_t v);
