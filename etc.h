// SORT
equation eqn(term a);
vec<term> flatten(tag t, term a);
set<term> freeVars(term a);
term imp(term a, term b);
bool occurs(term a, term b);
term quantify(term a);
clause uniq(const clause& c);
set<clause> uniq(const set<clause>& cs);
///

template <class T> void cartProduct(const vec<vec<T>>& vs, size_t i, vec<size_t>& js, vec<vec<T>>& rs) {
	if (i == js.size()) {
		vec<T> r;
		for (size_t i = 0; i != vs.size(); ++i) r.push_back(vs[i][js[i]]);
		rs.push_back(r);
		return;
	}
	for (js[i] = 0; js[i] != vs[i].size(); ++js[i]) cartProduct(vs, i + 1, js, rs);
}

template <class T> vec<vec<T>> cartProduct(const vec<vec<T>>& vs) {
	vec<size_t> js;
	for (auto& v: vs) js.push_back(0);
	vec<vec<T>> rs;
	cartProduct(vs, 0, js, rs);
	return rs;
}
