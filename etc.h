extern char buf[5000];

// The SZS ontologies for automated reasoning software, or at least, the subset thereof that is used here
enum {
#define _(x) z_##x,
#include "szs.h"
};

extern const char* szsNames[];

// SORT
void flatten(int tag, Ex* a, vector<Ex*>& r);
void freeVars(Ex* a, vec<Ex*> boundv, vec<Ex*>& freev);
Ex* imp(Ex* a, Ex* b);
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
