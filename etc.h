extern char buf[5000];

// SORT
void flatten(int tag, Ex* a, vector<Ex*>& r);
void freeVars(Ex* a, vec<Ex*> boundv, vec<Ex*>& freev);
Ex* imp(Ex* a, Ex* b);
void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d);
void mpz_ediv_r(mpz_t r, const mpz_t n, const mpz_t d);
void mpz_round(mpz_t q, mpz_t n, mpz_t d);
bool occurs(Ex* a, Ex* b);
Ex* quantify(Ex* a);
///

template <class T> void cartProduct(const vector<vector<T>>& vs, size_t i, vec<size_t>& js, vector<vector<T>>& rs) {
	if (i == js.size()) {
		vector<T> r;
		for (size_t i = 0; i < vs.size(); ++i) r.push_back(vs[i][js[i]]);
		rs.push_back(r);
		return;
	}
	for (js[i] = 0; js[i] != vs[i].size(); ++js[i]) cartProduct(vs, i + 1, js, rs);
}

template <class T> vector<vector<T>> cartProduct(const vector<vector<T>>& vs) {
	vec<size_t> js;
	for (auto& v: vs) js.add(0);
	vector<vector<T>> rs;
	cartProduct(vs, 0, js, rs);
	return rs;
}
