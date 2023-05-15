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
