struct Int: Expr {
	mpz_t v;

	Int(mpz_t v): Expr(Tag::integer) {
		memcpy(this->v, v, sizeof(mpz_t));
	}
};

Int* integer(mpz_t v);
