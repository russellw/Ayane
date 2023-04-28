// The SZS ontologies for automated reasoning software, or at least, the subset thereof that is used here
enum {
#define _(x) z_##x,
#include "szs.h"
};

extern const char* szsNames[];

// SORT
Eqn eqn(Term* a);
vec<Term*> flatten(int tag, Term* a);
set<Term*> freeVars(Term* a);
Term* imp(Term* a, Term* b);
bool occurs(Term* a, Term* b);
Term* quantify(Term* a);
clause uniq(Clause* c);
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
