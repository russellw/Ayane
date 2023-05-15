// Exit codes specified by this program have small negative numbers to avoid overlap with those specified by POSIX or Windows.
// Increasingly negative numbers are assigned roughly in order from 'things that can reasonably happen even when the input is not
// actually incorrect' (e.g. problem contains higher order logic that the system does not understand, 'inappropriate' in SZS
// terminology) to 'things that can only happen if there is an error in the input' (e.g. syntax error) to 'things that can only
// happen if there is a bug in the code' (e.g. assert failure).
const int inappropriateError = -1;
const int typeError = -2;
const int syntaxError = -3;
const int assertError = -4;

extern char buf[5000];

// SORT
void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d);
void mpz_ediv_r(mpz_t r, const mpz_t n, const mpz_t d);
void mpz_round(mpz_t q, const mpz_t n, const mpz_t d);
///

template <class T> void cartProduct(const vector<vector<T>>& vs, size_t i, Vec<size_t>& js, vector<vector<T>>& rs) {
	if (i == js.n) {
		vector<T> r;
		for (size_t i = 0; i < vs.size(); ++i) r.push_back(vs[i][js[i]]);
		rs.push_back(r);
		return;
	}
	for (js[i] = 0; js[i] != vs[i].size(); ++js[i]) cartProduct(vs, i + 1, js, rs);
}

template <class T> vector<vector<T>> cartProduct(const vector<vector<T>>& vs) {
	Vec<size_t> js;
	for (auto& v: vs) js.add(0);
	vector<vector<T>> rs;
	cartProduct(vs, 0, js, rs);
	return rs;
}
