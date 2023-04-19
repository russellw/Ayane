#include "main.h"

#ifdef DEBUG
namespace {
clause mkClause(vec<term>& neg, vec<term>& pos) {
	auto c = make_pair(neg, pos);
	neg.clear();
	pos.clear();
	return c;
}

void testSubsume() {
	clearStrings();
	vec<term> neg, pos;

	term a = gensym(kind::Individual);
	term a1 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual}));
	term b = gensym(kind::Individual);
	term p = gensym(kind::Bool);
	term p1 = gensym(type(kind::Fn, vec<type>{kind::Bool, kind::Individual}));
	term p2 = gensym(type(kind::Fn, vec<type>{kind::Bool, kind::Individual, kind::Individual}));
	term q = gensym(kind::Bool);
	term q1 = gensym(type(kind::Fn, vec<type>{kind::Bool, kind::Individual}));
	size_t i = 1000;
	auto x = var(i++, kind::Individual);
	auto y = var(i++, kind::Individual);

	clause c;
	clause d;

	// False <= false.
	c = mkClause(neg, pos);
	d = c;
	assert(subsumes(c, d));

	// False <= p.
	c = mkClause(neg, pos);
	pos.push_back(p);
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// p <= p.
	pos.push_back(p);
	c = mkClause(neg, pos);
	d = c;
	assert(subsumes(c, d));

	// !p <= !p.
	neg.push_back(p);
	c = mkClause(neg, pos);
	d = c;
	assert(subsumes(c, d));

	// p <= p | p.
	pos.push_back(p);
	c = mkClause(neg, pos);
	pos.push_back(p);
	pos.push_back(p);
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// p !<= !p.
	pos.push_back(p);
	c = mkClause(neg, pos);
	neg.push_back(p);
	d = mkClause(neg, pos);
	assert(!subsumes(c, d));
	assert(!subsumes(d, c));

	// p | q <= q | p.
	pos.push_back(p);
	pos.push_back(q);
	c = mkClause(neg, pos);
	pos.push_back(q);
	pos.push_back(p);
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(subsumes(d, c));

	// p | q <= p | q | p.
	pos.push_back(p);
	pos.push_back(q);
	c = mkClause(neg, pos);
	pos.push_back(p);
	pos.push_back(q);
	pos.push_back(p);
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// p(a) | p(b) | q(a) | q(b) | <= p(a) | q(a) | p(b) | q(b).
	pos.push_back(term(p1, a));
	pos.push_back(term(p1, b));
	pos.push_back(term(q1, a));
	pos.push_back(term(q1, b));
	c = mkClause(neg, pos);
	pos.push_back(term(p1, a));
	pos.push_back(term(q1, a));
	pos.push_back(term(p1, b));
	pos.push_back(term(q1, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(subsumes(d, c));

	// p(x,y) <= p(a,b).
	pos.push_back(term(p2, x, y));
	c = mkClause(neg, pos);
	pos.push_back(term(p2, a, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// p(x,x) !<= p(a,b).
	pos.push_back(term(p2, x, x));
	c = mkClause(neg, pos);
	pos.push_back(term(p2, a, b));
	d = mkClause(neg, pos);
	assert(!subsumes(c, d));
	assert(!subsumes(d, c));

	// p(x) <= p(y).
	pos.push_back(term(p1, x));
	c = mkClause(neg, pos);
	pos.push_back(term(p1, y));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(subsumes(d, c));

	// p(x) | p(a(x)) | p(a(a(x))) <= p(y) | p(a(y)) | p(a(a(y))).
	pos.push_back(term(p1, x));
	pos.push_back(term(p1, term(a1, x)));
	pos.push_back(term(p1, term(a1, term(a1, x))));
	c = mkClause(neg, pos);
	pos.push_back(term(p1, y));
	pos.push_back(term(p1, term(a1, y)));
	pos.push_back(term(p1, term(a1, term(a1, y))));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(subsumes(d, c));

	// p(x) | p(a) <= p(a) | p(b).
	pos.push_back(term(p1, x));
	pos.push_back(term(p1, a));
	c = mkClause(neg, pos);
	pos.push_back(term(p1, a));
	pos.push_back(term(p1, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// p(x) | p(a(x)) <= p(a(y)) | p(y).
	pos.push_back(term(p1, x));
	pos.push_back(term(p1, term(a1, x)));
	c = mkClause(neg, pos);
	pos.push_back(term(p1, term(a1, y)));
	pos.push_back(term(p1, y));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(subsumes(d, c));

	// p(x) | p(a(x)) | p(a(a(x))) <= p(a(a(y))) | p(a(y)) | p(y).
	pos.push_back(term(p1, x));
	pos.push_back(term(p1, term(a1, x)));
	pos.push_back(term(p1, term(a1, term(a1, x))));
	c = mkClause(neg, pos);
	pos.push_back(term(p1, term(a1, term(a1, y))));
	pos.push_back(term(p1, term(a1, y)));
	pos.push_back(term(p1, y));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(subsumes(d, c));

	// (a = x) <= (a = b).
	pos.push_back(term(tag::Eq, a, x));
	c = mkClause(neg, pos);
	pos.push_back(term(tag::Eq, a, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// (x = a) <= (a = b).
	pos.push_back(term(tag::Eq, x, a));
	c = mkClause(neg, pos);
	pos.push_back(term(tag::Eq, a, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// !p(y) | !p(x) | q(x) <= !p(a) | !p(b) | q(b).
	neg.push_back(term(p1, y));
	neg.push_back(term(p1, x));
	pos.push_back(term(q1, x));
	c = mkClause(neg, pos);
	neg.push_back(term(p1, a));
	neg.push_back(term(p1, b));
	pos.push_back(term(q1, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// !p(x) | !p(y) | q(x) <= !p(a) | !p(b) | q(b).
	neg.push_back(term(p1, x));
	neg.push_back(term(p1, y));
	pos.push_back(term(q1, x));
	c = mkClause(neg, pos);
	neg.push_back(term(p1, a));
	neg.push_back(term(p1, b));
	pos.push_back(term(q1, b));
	d = mkClause(neg, pos);
	assert(subsumes(c, d));
	assert(!subsumes(d, c));

	// p(x,a(x)) !<= p(a(y),a(y)).
	pos.push_back(term(p2, x, term(a1, x)));
	c = mkClause(neg, pos);
	pos.push_back(term(p2, term(a1, y), term(a1, y)));
	d = mkClause(neg, pos);
	assert(!subsumes(c, d));
	assert(!subsumes(d, c));
}

void cnf(term a, set<clause>& cs) {
	set<term> initialFormulas;
	initialFormulas.add(a);
	ProofCnf proofCnf;
	cnf(initialFormulas, proofCnf, cs);
}

void testCnf() {
	set<clause> cs;
	set<clause> cs1;
	int i = 0;

	// False.
	cs.clear();
	cs1.clear();
	cnf(tag::False, cs);
	assert(cs.size() == 1);
	assert(*cs.begin() == falsec);

	// True.
	cs.clear();
	cs1.clear();
	cnf(tag::True, cs);
	assert(cs.size() == 0);

	term a = gensym(kind::Bool);

	// a
	cs.clear();
	cs1.clear();
	cnf(a, cs);
	assert(cs.size() == 1);
	auto c = *cs.begin();
	assert(c.first.empty());
	assert(c.second == vec<term>{a});

	// !a.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, a), cs);
	assert(cs.size() == 1);
	c = *cs.begin();
	assert(c.first == vec<term>{a});
	assert(c.second.empty());

	// !!a.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, term(tag::Not, a)), cs);
	assert(cs.size() == 1);
	c = *cs.begin();
	assert(c.first.empty());
	assert(c.second == vec<term>{a});

	// !true.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, tag::True), cs);
	assert(cs.size() == 1);
	assert(*cs.begin() == falsec);

	// !false.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, tag::False), cs);
	assert(cs.size() == 0);

	term b = gensym(kind::Bool);

	// a || b.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Or, a, b), cs);
	assert(cs.size() == 1);
	c = *cs.begin();
	assert(c.first.empty());
	assert((c.second == vec<term>{a, b}));

	// a && b.
	cs.clear();
	cs1.clear();
	cnf(term(tag::And, a, b), cs);
	assert(cs.size() == 2);
	cs1.add(make_pair(vec<term>(), vec<term>{a}));
	cs1.add(make_pair(vec<term>(), vec<term>{b}));
	assert(cs == cs1);

	term a1 = gensym(kind::Bool);
	term a2 = gensym(kind::Bool);
	term a3 = gensym(kind::Bool);

	// a1 || a2 || a3.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Or, a1, a2, a3), cs);
	assert(cs.size() == 1);
	c = *cs.begin();
	assert(c.first.empty());
	assert((c.second == vec<term>{a1, a2, a3}));

	// (a1 || a2) || a3.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Or, term(tag::Or, a1, a2), a3), cs);
	assert(cs.size() == 1);
	c = *cs.begin();
	assert(c.first.empty());
	assert((c.second == vec<term>{a1, a2, a3}));

	// a1 || (a2 || a3).
	cs.clear();
	cs1.clear();
	cnf(term(tag::Or, a1, term(tag::Or, a2, a3)), cs);
	assert(cs.size() == 1);
	c = *cs.begin();
	assert(c.first.empty());
	assert((c.second == vec<term>{a1, a2, a3}));

	// a1 && a2 && a3.
	cs.clear();
	cs1.clear();
	cnf(term(tag::And, a1, a2, a3), cs);
	assert(cs.size() == 3);
	cs1.add(make_pair(vec<term>(), vec<term>{a1}));
	cs1.add(make_pair(vec<term>(), vec<term>{a2}));
	cs1.add(make_pair(vec<term>(), vec<term>{a3}));
	assert(cs == cs1);

	// (a1 && a2) && a3.
	cs.clear();
	cs1.clear();
	cnf(term(tag::And, term(tag::And, a1, a2), a3), cs);
	assert(cs.size() == 3);
	cs1.add(make_pair(vec<term>(), vec<term>{a1}));
	cs1.add(make_pair(vec<term>(), vec<term>{a2}));
	cs1.add(make_pair(vec<term>(), vec<term>{a3}));
	assert(cs == cs1);

	// a1 && (a2 && a3).
	cs.clear();
	cs1.clear();
	cnf(term(tag::And, a1, term(tag::And, a2, a3)), cs);
	assert(cs.size() == 3);
	cs1.add(make_pair(vec<term>(), vec<term>{a1}));
	cs1.add(make_pair(vec<term>(), vec<term>{a2}));
	cs1.add(make_pair(vec<term>(), vec<term>{a3}));
	assert(cs == cs1);

	// !(a1 || a2 || a3).
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, term(tag::Or, a1, a2, a3)), cs);
	assert(cs.size() == 3);
	cs1.add(make_pair(vec<term>{a1}, vec<term>()));
	cs1.add(make_pair(vec<term>{a2}, vec<term>()));
	cs1.add(make_pair(vec<term>{a3}, vec<term>()));
	assert(cs == cs1);

	// !(a1 && a2 && a3).
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, term(tag::And, a1, a2, a3)), cs);
	assert(cs.size() == 1);
	cs1.add(make_pair(vec<term>{a1, a2, a3}, vec<term>()));
	assert(cs == cs1);

	// False <=> false.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Eqv, tag::False, tag::False), cs);
	assert(cs.size() == 0);

	// True <=> true.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Eqv, tag::True, tag::True), cs);
	assert(cs.size() == 0);

	// False <=> true.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Eqv, tag::False, tag::True), cs);
	assert(cs.size() == 1);
	assert(*cs.begin() == falsec);

	// a <=> a.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Eqv, a, a), cs);
	assert(cs.size() == 0);

	// a => a.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Or, term(tag::Not, a), a), cs);
	assert(cs.size() == 0);

	term b1 = gensym(kind::Bool);
	term b2 = gensym(kind::Bool);
	term b3 = gensym(kind::Bool);

	// a || (b1 && b2 && b3).
	cs.clear();
	cs1.clear();
	cnf(term(tag::Or, a, term(tag::And, b1, b2, b3)), cs);
	assert(cs.size() == 3);
	cs1.add(make_pair(vec<term>(), vec<term>{a, b1}));
	cs1.add(make_pair(vec<term>(), vec<term>{a, b2}));
	cs1.add(make_pair(vec<term>(), vec<term>{a, b3}));
	assert(cs == cs1);

	term p = gensym(type(kind::Fn, vec<type>{kind::Bool, kind::Individual, kind::Individual, kind::Individual}));
	term s1 = gensym(kind::Individual);
	term s2 = gensym(kind::Individual);
	term s3 = gensym(kind::Individual);

	// p(s1, s2, s3).
	cs.clear();
	cs1.clear();
	cnf(term(p, s1, s2, s3), cs);
	assert(cs.size() == 1);
	cs1.add(make_pair(vec<term>(), vec<term>{term(p, s1, s2, s3)}));
	assert(cs == cs1);

	// !p(s1, s2, s3).
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, term(p, s1, s2, s3)), cs);
	assert(cs.size() == 1);
	cs1.add(make_pair(vec<term>{term(p, s1, s2, s3)}, vec<term>()));
	assert(cs == cs1);

	term n1 = gensym(kind::Integer);
	term n2 = gensym(kind::Integer);
	term n3 = gensym(kind::Integer);

	// n1 == 42.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Eq, n1, integer(42)), cs);
	assert(cs.size() == 1);
	cs1.add(make_pair(vec<term>(), vec<term>{term(tag::Eq, n1, integer(42))}));
	assert(cs == cs1);

	// n1 + n2 + n3 != 99.
	cs.clear();
	cs1.clear();
	cnf(term(tag::Not, term(tag::Eq, term(tag::Add, term(tag::Add, n1, n2), n3), integer(99))), cs);
	assert(cs.size() == 1);
	cs1.add(make_pair(vec<term>{term(tag::Eq, term(tag::Add, term(tag::Add, n1, n2), n3), integer(99))}, vec<term>()));
	assert(cs == cs1);
}

void testReal(const char* s, int n, unsigned d) {
	term a = real(s)[1];
	term b = rational(n, d);
	assert(a == b);
}

size_t sum(const vec<size_t>& v) {
	size_t n = 0;
	for (auto& a: v) n += a;
	return n;
}

void testGraph1() {
	// https://tanujkhattar.wordpress.com/2016/01/11/dominator-tree-of-a-directed-graph/
	vec<int> xs;
	xs.push_back('a');
	xs.push_back('b');
	xs.push_back('c');
	xs.push_back('d');
	xs.push_back('e');
	xs.push_back('f');
	xs.push_back('g');
	xs.push_back('h');
	xs.push_back('i');
	xs.push_back('j');
	xs.push_back('k');
	xs.push_back('l');
	xs.push_back('r');

	graph<int> g;
	g.add(make_pair('a', 'd'));
	g.add(make_pair('b', 'a'));
	g.add(make_pair('b', 'd'));
	g.add(make_pair('b', 'e'));
	g.add(make_pair('c', 'f'));
	g.add(make_pair('c', 'g'));
	g.add(make_pair('d', 'l'));
	g.add(make_pair('e', 'h'));
	g.add(make_pair('f', 'i'));
	g.add(make_pair('g', 'i'));
	g.add(make_pair('g', 'j'));
	g.add(make_pair('h', 'e'));
	g.add(make_pair('h', 'k'));
	g.add(make_pair('i', 'k'));
	g.add(make_pair('j', 'i'));
	g.add(make_pair('k', 'i'));
	g.add(make_pair('k', 'r'));
	g.add(make_pair('l', 'h'));
	g.add(make_pair('r', 'a'));
	g.add(make_pair('r', 'b'));
	g.add(make_pair('r', 'c'));
	assert(nodes(g).size() == xs.size());

	int s = 'r';

	// a dominates a.
	for (auto& a: xs) assert(dominates(g, s, a, a));

	// s dominates a.
	for (auto& a: xs) assert(dominates(g, s, s, a));

	// Immediate dominators.
	for (auto& b: xs) {
		auto a = idom(g, s, b);
		switch (b) {
		case 'f':
		case 'g':
			assert(a == 'c');
			break;
		case 'j':
			assert(a == 'g');
			break;
		case 'l':
			assert(a == 'd');
			break;
		case 'r':
			assert(!a);
			break;
		default:
			assert(a == 'r');
			break;
		}
	}
}

void testGraph2() {
	// Tiger book page 439.
	graph<int> g;
	g.add(make_pair(1, 2));
	g.add(make_pair(1, 5));
	g.add(make_pair(1, 9));
	g.add(make_pair(2, 3));
	g.add(make_pair(3, 3));
	g.add(make_pair(3, 4));
	g.add(make_pair(4, 13));
	g.add(make_pair(5, 6));
	g.add(make_pair(5, 7));
	g.add(make_pair(6, 4));
	g.add(make_pair(6, 8));
	g.add(make_pair(7, 8));
	g.add(make_pair(7, 12));
	g.add(make_pair(8, 13));
	g.add(make_pair(8, 5));
	g.add(make_pair(9, 10));
	g.add(make_pair(9, 11));
	g.add(make_pair(10, 12));
	g.add(make_pair(11, 12));
	g.add(make_pair(12, 13));
	assert(nodes(g).size() == 13);

	int s = 1;

	// a dominates a.
	for (int a = 1; a <= 13; ++a) assert(dominates(g, s, a, a));

	// s dominates a.
	for (int a = 1; a <= 13; ++a) assert(dominates(g, s, s, a));

	// Dominance frontier.
	set<int> r;
	r.add(4);
	r.add(5);
	r.add(12);
	r.add(13);
	assert(domFrontier(g, s, 5) == r);
}

void testGraph3() {
	// https://users.aalto.fi/~tjunttil/2020-DP-AUT/notes-sat/cdcl.html
	graph<int> g;
	g.add(make_pair(1, 2));
	g.add(make_pair(1, 3));
	g.add(make_pair(2, 5));
	g.add(make_pair(2, 8));
	g.add(make_pair(3, 4));
	g.add(make_pair(4, 5));
	g.add(make_pair(5, 7));
	g.add(make_pair(6, 7));
	g.add(make_pair(7, 8));
	g.add(make_pair(8, 9));
	g.add(make_pair(8, 10));
	g.add(make_pair(9, 11));
	g.add(make_pair(10, 11));
	g.add(make_pair(10, 12));
	g.add(make_pair(11, 0));
	g.add(make_pair(12, 0));
	assert(nodes(g).size() == 13);

	// First UIP.
	assert(idom(g, 6, 0) == 8);

	set<int> r;
	r.add(7);
	r.add(8);
	r.add(9);
	r.add(10);
	r.add(11);
	r.add(12);
	r.add(0);
	assert(transSuccessors(g, 6) == r);
}

term replace(const map<term, term>& m, term a) {
	if (m.count(a)) {
		assert(tag(a) == tag::Var);
		return replace(m, m.at(a));
	}

	auto n = a.size();
	vec<term> v(1, a[0]);
	for (size_t i = 1; i != n; ++i) v.push_back(replace(m, a[i]));
	return term(v);
}

void testMatch() {
	// Subset of unify where only the first argument can be treated as a variable to be matched against the second argument. Applied
	// to the same test cases as unify, gives the same results in some cases, but different results in others. In particular, has no
	// notion of an occurs check; in actual use, it is assumed that the arguments will have disjoint variables.
	size_t i = 1000;

	term a = gensym(kind::Individual);
	term b = gensym(kind::Individual);

	term f1 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual}));
	term f2 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual, kind::Individual}));
	term g1 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual}));

	auto x = var(i++, kind::Individual);
	auto y = var(i++, kind::Individual);
	auto z = var(i++, kind::Individual);

	map<term, term> m;

	// Succeeds. (tautology).
	m.clear();
	assert(match(m, a, a));
	assert(m.size() == 0);

	// a and b do not match.
	m.clear();
	assert(!match(m, a, b));

	// Succeeds. (tautology).
	m.clear();
	assert(match(m, x, x));
	assert(m.size() == 1);
	assert(m.at(x) == x);

	// x is not matched with the constant a, because the variable is on the right-hand side.
	m.clear();
	assert(!match(m, a, x));

	// x and y are aliased.
	m.clear();
	assert(match(m, x, y));
	assert(m.size() == 1);
	assert(replace(m, x) == replace(m, y));

	// Function and constant symbols match, x is unified with the constant b.
	m.clear();
	assert(match(m, term(f2, a, x), term(f2, a, b)));
	assert(m.size() == 1);
	assert(replace(m, x) == b);

	// f and g do not match.
	m.clear();
	assert(!match(m, term(f1, a), term(g1, a)));

	// x and y are aliased.
	m.clear();
	assert(match(m, term(f1, x), term(f1, y)));
	assert(m.size() == 1);
	assert(replace(m, x) == replace(m, y));

	// f and g do not match.
	m.clear();
	assert(!match(m, term(f1, x), term(g1, y)));

	// Fails. The f function symbols have different arity.
	m.clear();
	assert(!match(m, term(f1, x), term(f2, y, z)));

	// Does not match y with the term g(x), because the variable is on the right-hand side.
	m.clear();
	assert(!match(m, term(f1, term(g1, x)), term(f1, y)));

	// Does not match, because the variable is on the right-hand side.
	m.clear();
	assert(!match(m, term(f2, term(g1, x), x), term(f2, y, a)));

	// Returns false in first-order logic and many modern Prolog dialects (enforced by the occurs check) but returns true here
	// because match has no notion of an occurs check.
	m.clear();
	assert(match(m, x, term(f1, x)));
	assert(m.size() == 1);

	// Both x and y are unified with the constant a.
	m.clear();
	assert(match(m, x, y));
	assert(match(m, y, a));
	assert(m.size() == 2);
	assert(replace(m, x) == a);
	assert(replace(m, y) == a);

	// Fails this time, because the variable is on the right-hand side.
	m.clear();
	assert(!match(m, a, y));

	// Fails. a and b do not match, so x can't be unified with both.
	m.clear();
	assert(match(m, x, a));
	assert(!match(m, b, x));
}

bool unify(map<termx, termx>& m, term a, term b) {
	return unify(m, a, 0, b, 0);
}

term replace(const map<termx, termx>& m, term a) {
	return replace(m, a, 0);
}

void testUnify() {
	// https://en.wikipedia.org/wiki/Unification_(computer_science)#Examples_of_syntactic_unification_of_first-order_terms
	size_t i = 1000;

	term a = gensym(kind::Individual);
	term b = gensym(kind::Individual);

	term f1 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual}));
	term f2 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual, kind::Individual}));
	term g1 = gensym(type(kind::Fn, vec<type>{kind::Individual, kind::Individual}));

	auto x = var(i++, kind::Individual);
	auto y = var(i++, kind::Individual);
	auto z = var(i++, kind::Individual);

	map<termx, termx> m;

	// Succeeds. (tautology).
	m.clear();
	assert(unify(m, a, a));
	assert(m.size() == 0);

	// a and b do not match.
	m.clear();
	assert(!unify(m, a, b));

	// Succeeds. (tautology).
	m.clear();
	assert(unify(m, x, x));
	assert(m.size() == 0);

	// x is unified with the constant a.
	m.clear();
	assert(unify(m, a, x));
	assert(m.size() == 1);
	assert(replace(m, x) == a);

	// x and y are aliased.
	m.clear();
	assert(unify(m, x, y));
	assert(m.size() == 1);
	assert(replace(m, x) == replace(m, y));

	// Function and constant symbols match, x is unified with the constant b.
	m.clear();
	assert(unify(m, term(f2, a, x), term(f2, a, b)));
	assert(m.size() == 1);
	assert(replace(m, x) == b);

	// f and g do not match.
	m.clear();
	assert(!unify(m, term(f1, a), term(g1, a)));

	// x and y are aliased.
	m.clear();
	assert(unify(m, term(f1, x), term(f1, y)));
	assert(m.size() == 1);
	assert(replace(m, x) == replace(m, y));

	// f and g do not match.
	m.clear();
	assert(!unify(m, term(f1, x), term(g1, y)));

	// Fails. The f function symbols have different arity.
	m.clear();
	assert(!unify(m, term(f1, x), term(f2, y, z)));

	// Unifies y with the term g1(x).
	m.clear();
	assert(unify(m, term(f1, term(g1, x)), term(f1, y)));
	assert(m.size() == 1);
	assert(replace(m, y) == term(g1, x));

	// Unifies x with constant a, and y with the term g1(a).
	m.clear();
	assert(unify(m, term(f2, term(g1, x), x), term(f2, y, a)));
	assert(m.size() == 2);
	assert(replace(m, x) == a);
	assert(replace(m, y) == term(g1, a));

	// Returns false in first-order logic and many modern Prolog dialects (enforced by the occurs check).
	m.clear();
	assert(!unify(m, x, term(f1, x)));

	// Both x and y are unified with the constant a.
	m.clear();
	assert(unify(m, x, y));
	assert(unify(m, y, a));
	assert(m.size() == 2);
	assert(replace(m, x) == a);
	assert(replace(m, y) == a);

	// As above (order of equations in set doesn't matter).
	m.clear();
	assert(unify(m, a, y));
	assert(unify(m, x, y));
	assert(m.size() == 2);
	assert(replace(m, x) == a);
	assert(replace(m, y) == a);

	// Fails. a and b do not match, so x can't be unified with both.
	m.clear();
	assert(unify(m, x, a));
	assert(!unify(m, b, x));
}

bool eq(const vec<int>& q, int x, int y, int z) {
	if (q.size() != 3) return 0;
	if (q[0] != x) return 0;
	if (q[1] != y) return 0;
	if (q[2] != z) return 0;
	return 1;
}

void testCartProduct() {
	int a0 = 100;
	int a1 = 101;
	int b0 = 200;
	int b1 = 201;
	int b2 = 202;
	int c0 = 300;
	int c1 = 301;
	int c2 = 302;
	int c3 = 303;
	vec<vec<int>> qs;
	vec<int> q;
	q.clear();
	q.push_back(a0);
	q.push_back(a1);
	qs.push_back(q);
	q.clear();
	q.push_back(b0);
	q.push_back(b1);
	q.push_back(b2);
	qs.push_back(q);
	q.clear();
	q.push_back(c0);
	q.push_back(c1);
	q.push_back(c2);
	q.push_back(c3);
	qs.push_back(q);
	auto rs = cartProduct(qs);
	size_t i = 0;
	assert(eq(rs[i++], a0, b0, c0));
	assert(eq(rs[i++], a0, b0, c1));
	assert(eq(rs[i++], a0, b0, c2));
	assert(eq(rs[i++], a0, b0, c3));
	assert(eq(rs[i++], a0, b1, c0));
	assert(eq(rs[i++], a0, b1, c1));
	assert(eq(rs[i++], a0, b1, c2));
	assert(eq(rs[i++], a0, b1, c3));
	assert(eq(rs[i++], a0, b2, c0));
	assert(eq(rs[i++], a0, b2, c1));
	assert(eq(rs[i++], a0, b2, c2));
	assert(eq(rs[i++], a0, b2, c3));
	assert(eq(rs[i++], a1, b0, c0));
	assert(eq(rs[i++], a1, b0, c1));
	assert(eq(rs[i++], a1, b0, c2));
	assert(eq(rs[i++], a1, b0, c3));
	assert(eq(rs[i++], a1, b1, c0));
	assert(eq(rs[i++], a1, b1, c1));
	assert(eq(rs[i++], a1, b1, c2));
	assert(eq(rs[i++], a1, b1, c3));
	assert(eq(rs[i++], a1, b2, c0));
	assert(eq(rs[i++], a1, b2, c1));
	assert(eq(rs[i++], a1, b2, c2));
	assert(eq(rs[i++], a1, b2, c3));
}

void testDpll() {
	set<clause> cs;
	map<term, term> m;

	clearStrings();
	term a(intern("a"), kind::Bool);
	term b = gensym(kind::Bool);

	// No clauses.
	cs.clear();
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.empty());

	// False.
	cs.clear();
	cs.add(falsec);
	m.clear();
	assert(dpll(m, cs) == szs::Unsatisfiable);

	// False.
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{tag::False}));
	m.clear();
	assert(dpll(m, cs) == szs::Unsatisfiable);

	// False | false.
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{tag::False, tag::False}));
	m.clear();
	assert(dpll(m, cs) == szs::Unsatisfiable);

	// !true.
	cs.clear();
	cs.add(make_pair(vec<term>{tag::True}, vec<term>()));
	m.clear();
	assert(dpll(m, cs) == szs::Unsatisfiable);

	// True.
	cs.clear();
	cs.add(truec);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.empty());

	// True | true.
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{tag::True, tag::True}));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.empty());

	// !false.
	cs.clear();
	cs.add(make_pair(vec<term>{tag::False}, vec<term>()));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.empty());

	// a
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{a}));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 1);
	assert(m.at(a) == tag::True);

	// a | a.
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{a, a}));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 1);
	assert(m.at(a) == tag::True);

	// !a.
	cs.clear();
	cs.add(make_pair(vec<term>{a}, vec<term>()));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 1);
	assert(m.at(a) == tag::False);

	// !a | a.
	cs.clear();
	cs.add(make_pair(vec<term>{a}, vec<term>{a}));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);

	// !a & a.
	cs.clear();
	cs.add(make_pair(vec<term>{a}, vec<term>()));
	cs.add(make_pair(vec<term>(), vec<term>{a}));
	m.clear();
	assert(dpll(m, cs) == szs::Unsatisfiable);

	// b
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{b}));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 1);
	assert(m.at(b) == tag::True);

	// a & b.
	cs.clear();
	cs.add(make_pair(vec<term>(), vec<term>{a}));
	cs.add(make_pair(vec<term>(), vec<term>{b}));
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 2);
	assert(m.at(a) == tag::True);
	assert(m.at(b) == tag::True);

	// Cnf.

	// True <=> (true <=> true).
	cs.clear();
	cnf(term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, tag::True)), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);

	// True <=> (true <=> (true <=> true)).
	cs.clear();
	cnf(term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, tag::True))), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);

	// True <=> (true <=> (true <=> (true <=> true))).
	cs.clear();
	cnf(term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, tag::True)))), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);

	// True <=> (true <=> (true <=> (true <=> (true <=> true)))).
	cs.clear();
	cnf(term(
			tag::Eqv,
			tag::True,
			term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, tag::True))))),
		cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);

	// True <=> (true <=> (true <=> (true <=> (true <=> (true <=> true))))).
	cs.clear();
	cnf(term(
			tag::Eqv,
			tag::True,
			term(
				tag::Eqv,
				tag::True,
				term(
					tag::Eqv,
					tag::True,
					term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, term(tag::Eqv, tag::True, tag::True)))))),
		cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);

	int i = 1;
	term p = gensym(kind::Bool);
	term q = gensym(kind::Bool);
	term r = gensym(kind::Bool);

	// p & q & r.
	cs.clear();
	cnf(term(tag::And, p, q, r), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 3);
	assert(m.at(p) == tag::True);
	assert(m.at(q) == tag::True);
	assert(m.at(r) == tag::True);

	// (p & q) & r.
	cs.clear();
	cnf(term(tag::And, term(tag::And, p, q), r), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 3);
	assert(m.at(p) == tag::True);
	assert(m.at(q) == tag::True);
	assert(m.at(r) == tag::True);

	// p & (q & r).
	cs.clear();
	cnf(term(tag::And, p, term(tag::And, q, r)), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size() == 3);
	assert(m.at(p) == tag::True);
	assert(m.at(q) == tag::True);
	assert(m.at(r) == tag::True);

	// p | q | r.
	cs.clear();
	cnf(term(tag::Or, p, q, r), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size());

	// (p | q) | r.
	cs.clear();
	cnf(term(tag::Or, term(tag::Or, p, q), r), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size());

	// p | (q | r).
	cs.clear();
	cnf(term(tag::Or, p, term(tag::Or, q, r)), cs);
	m.clear();
	assert(dpll(m, cs) == szs::Satisfiable);
	assert(m.size());
}
} // namespace

void test() {
	static_assert(sizeof(type) == 4);
	static_assert(sizeof(term) == 4);

	// Strings.
	assert(keyword(intern("h")) == s_h);
	assert(intern("abcdef", 6) == intern("abcdef"));

	// Terms.
	term x = var(0, kind::Individual);
	term y = var(0, kind::Individual);
	term z = var(2, kind::Individual);

	assert(tag(x) == tag::Var);
	assert(x == y);
	assert(x != z);

	term a = term(tag::And, tag::True, tag::True);
	assert(a[1] == a[1]);
	assert(a.size() == 3);

	term b = term(tag::And, tag::True, tag::True);
	assert(a == b);

	// Collections.
	vec<term> v;
	v.push_back(x);
	v.push_back(y);
	v.push_back(z);
	assert(v[0] == x);
	assert(v[1] == y);
	assert(v[2] == z);
	heap->check();

	vec<term> u;
	u.push_back(x);
	u.push_back(y);
	u.push_back(z);
	assert(u == v);

	vec<size_t> vi;
	vi.push_back(1);
	vi.push_back(2);
	vi.push_back(3);

	vec<size_t> vi2;
	vi2.push_back(1);
	vi2.push_back(2);
	vi2.push_back(2);

	set<vec<size_t>> vis;
	vis.add(vi);
	vis.add(vi2);
	assert(vis.size() == 2);

	set<vec<term>> vs;
	vs.add(v);
	vs.add(u);
	assert(vs.size() == 1);

	set<term> s;
	s.add(x);
	s.add(y);
	s.add(z);
	assert(s.size() == 2);
	s.add(a);
	s.add(b);
	assert(s.size() == 3);

	set<vec<term>> vsu;
	vsu.add(v);
	vsu.add(u);
	assert(vsu.size() == 1);

	set<size_t> ints;
	ints.add(vi.begin(), vi.end());
	ints.add(vi2.begin(), vi2.end());
	assert(ints.size() == 3);
	heap->check();

	{
		auto ints3 = ints;

		set<size_t> ints2;
		ints2.add(1);
		ints2.add(2);

		assert(ints == ints3);
		assert(ints != ints2);
		ints.erase(3);
		assert(ints != ints3);
		assert(ints == ints2);

		ints = ints2;
		assert(ints != ints3);
		assert(ints == ints2);

		ints = ints3;
		assert(ints == ints3);
		assert(ints != ints2);

		ints = ints2;
		assert(ints != ints3);
		assert(ints == ints2);

		ints = ints3;
		assert(ints == ints3);
		assert(ints != ints2);
	}
	heap->check();

	{
		map<int, int> m;
		assert(m.empty());
		m.add(100, 1);
		m.add(200, 2);
		m.add(300, 3);
		m.add(400, 4);
		m.add(500, 5);
		assert(m.size() == 5);
		assert(m.at(100) == 1);
		assert(m.at(200) == 2);
		assert(m.at(300) == 3);
		assert(m.at(400) == 4);
		assert(m.at(500) == 5);
		m.add(100, 1);
		m.add(200, 2);
		m.add(300, 3);
		m.add(400, 4);
		m.add(500, 5);
		assert(m.size() == 5);

		map<int, int> m2;
		m2.add(500, 5);
		m2.add(400, 4);
		m2.add(300, 3);
		m2.add(200, 2);
		m2.add(100, 1);
		assert(m == m2);
		int i;
		assert(m2.get(100, i) && i == 1);
		assert(m2.get(200, i) && i == 2);
		assert(m2.get(300, i) && i == 3);
		assert(m2.get(400, i) && i == 4);
		assert(m2.get(500, i) && i == 5);
		assert(!m2.get(600, i));

		map<int, int> m3;
		auto& i1 = m3.gadd(100);
		assert(i1 == 0);
		i1 = 1;
		auto& i2 = m3.gadd(200);
		assert(i2 == 0);
		i2 = 1;
		auto& i3 = m3.gadd(300);
		assert(i3 == 0);
		i3 = 1;
		auto& i4 = m3.gadd(400);
		assert(i4 == 0);
		i4 = 1;
		auto& i5 = m3.gadd(500);
		assert(i5 == 0);
		i5 = 1;
		assert(m == m3);

		map<int, int> m4(m);
		assert(m == m4);

		auto m5 = m;
		assert(m == m5);

		m4 = m5;
		assert(m4 == m5);

		int n1 = 0;
		int n2 = 0;
		for (auto& kv: m) {
			n1 += kv.first;
			n2 += kv.second;
		}
		assert(n1 == 1500);
		assert(n2 == 15);
	}
	heap->check();

	{
		uint16_t i2 = 10;
		assert(hash(i2) == hash(i2));

		uint32_t i4 = 10;
		assert(hash(i4) == hash(i4));

		uint64_t i8 = 10;
		assert(hash(i8) == hash(i8));

		int i = 10;
		assert(hash(i) == hash(i));
	}

	{
		vec<char> v;
		for (int i = 0; i < 10; ++i) v.push_back(i);
		assert(v.size() == 10);
		assert(v.end() - v.begin() == 10);
		for (int i = 0; i < 10; ++i) assert(v[i] == i);
		int j = 0;
		for (auto i: v) j += i;
		assert(j == 45);

		set<char> s;
		for (int i = 0; i < 10; ++i) s.add(i);
		assert(s.size() == 10);
		for (int i = 0; i < 10; ++i) assert(s.count(i));
		j = 0;
		for (auto i: s) j += i;
		assert(j == 45);

		map<char, char> m;
		for (int i = 0; i < 10; ++i) m.gadd(i) = i + 19;
		for (int i = 0; i < 10; ++i) assert(m.gadd(i) == i + 19);
		j = 0;
		for (auto kv: m) j += kv.first;
		assert(j == 45);
	}
	heap->check();

	// Numbers.
	term one = integer(1);
	assert(one == integer(1));
	assert(one != integer(2));

	assert(integer("1000") == integer(1000));
	assert(integer("1000000000000") + integer("1000000000000") == integer("2000000000000"));
	assert(integer("1000000000000") - integer("1000000000000") == integer("0"));
	assert(integer("1000000000000") * integer("1000000000000") == integer("1000000000000000000000000"));
	assert(integer("1000000000000") / integer("1000000000000") == integer(1));

	assert(rational("-1/3") == rational(-1, 3));
	assert(rational("1/2") + rational("1/3") == rational("5/6"));
	assert(rational("1/2") - rational("1/3") == rational("1/6"));
	assert(rational("1/2") * rational("1/7") == rational("1/14"));
	assert(rational("1/2") / rational("1/7") == rational("7/2"));

	assert(integer(-3) < (integer(2)));

	assert(integer(1) < (integer(2)));
	assert(!(integer(1) < (integer(1))));
	assert(!(integer(2) < (integer(1))));

	assert(integer(1) <= (integer(2)));
	assert(integer(1) <= (integer(1)));
	assert(!(integer(2) <= (integer(1))));

	assert(rational("-3/10") < (rational("2/10")));

	assert(rational(1, 7) < (rational(2, 7)));
	assert(!(rational(1, 7) < (rational(1, 7))));
	assert(!(rational(2, 7) < (rational(1, 7))));

	assert(rational(1, 7) <= (rational(2, 7)));
	assert(rational(1, 7) <= (rational(1, 7)));
	assert(!(rational(2, 7) <= (rational(1, 7))));

	assert(-integer(5) == integer(-5));
	assert(-rational(5, 99) == rational(-5, 99));

	// Sets.
	set<size_t> ints1;
	ints1.add(1);
	ints1.add(2);
	ints1.add(3);

	set<size_t> ints2;
	ints2.add(1);
	ints2.add(3);
	ints2.add(2);

	assert(ints1 == ints2);

	set<term> vals1;
	vals1.add(integer(100));
	vals1.add(integer(100));
	assert(vals1.size() == 1);
	vals1.add(var(100, kind::Individual));
	vals1.add(var(100, kind::Individual));
	assert(vals1.size() == 2);

	// Graphs.
	testGraph1();
	testGraph2();
	testGraph3();

	// Types.
	map<term, term> types;
	assert(type(term(tag::True)) == kind::Bool);
	assert(type(integer(11)) == kind::Integer);
	assert(type(rational(1, 1)) == kind::Rational);
	assert(type(var(100, kind::Individual)) == kind::Individual);
	assert(type(term(tag::And, tag::False, tag::True)) == kind::Bool);

	// Eval.
	map<term, term> m;
	assert(simplify(m, term(tag::Eq, integer(3), integer(3))) == tag::True);
	assert(simplify(m, term(tag::Eq, integer(3), integer(4))) == tag::False);
	assert(simplify(m, term(tag::IsInteger, integer(3))) == tag::True);
	assert(simplify(m, term(tag::IsInteger, rational(3, 1))) == tag::True);
	assert(simplify(m, term(tag::IsInteger, rational(3, 2))) == tag::False);

	assert(simplify(m, term(tag::DivF, integer(5), integer(3))) == integer(1));
	assert(simplify(m, term(tag::DivF, integer(-5), integer(3))) == integer(-2));
	assert(simplify(m, term(tag::DivF, integer(5), integer(-3))) == integer(-2));
	assert(simplify(m, term(tag::DivF, integer(-5), integer(-3))) == integer(1));
	assert(simplify(m, term(tag::DivF, rational(5, 1), rational(3, 1))) == rational(1, 1));
	assert(simplify(m, term(tag::DivF, rational(-5, 1), rational(3, 1))) == rational(-2, 1));
	assert(simplify(m, term(tag::DivF, rational(5, 1), rational(-3, 1))) == rational(-2, 1));
	assert(simplify(m, term(tag::DivF, rational(-5, 1), rational(-3, 1))) == rational(1, 1));

	assert(simplify(m, term(tag::RemF, integer(5), integer(3))) == integer(2));
	assert(simplify(m, term(tag::RemF, integer(-5), integer(3))) == integer(1));
	assert(simplify(m, term(tag::RemF, integer(5), integer(-3))) == integer(-1));
	assert(simplify(m, term(tag::RemF, integer(-5), integer(-3))) == integer(-2));
	assert(simplify(m, term(tag::RemF, rational(5, 1), rational(3, 1))) == rational(2, 1));
	assert(simplify(m, term(tag::RemF, rational(-5, 1), rational(3, 1))) == rational(1, 1));
	assert(simplify(m, term(tag::RemF, rational(5, 1), rational(-3, 1))) == rational(-1, 1));
	assert(simplify(m, term(tag::RemF, rational(-5, 1), rational(-3, 1))) == rational(-2, 1));

	assert(simplify(m, term(tag::DivT, integer(5), integer(3))) == integer(5 / 3));
	assert(simplify(m, term(tag::DivT, integer(-5), integer(3))) == integer(-5 / 3));
	assert(simplify(m, term(tag::DivT, integer(5), integer(-3))) == integer(5 / -3));
	assert(simplify(m, term(tag::DivT, integer(-5), integer(-3))) == integer(-5 / -3));
	assert(simplify(m, term(tag::DivT, integer(5), integer(3))) == integer(1));
	assert(simplify(m, term(tag::DivT, integer(-5), integer(3))) == integer(-1));
	assert(simplify(m, term(tag::DivT, integer(5), integer(-3))) == integer(-1));
	assert(simplify(m, term(tag::DivT, integer(-5), integer(-3))) == integer(1));
	assert(simplify(m, term(tag::DivT, rational(5, 1), rational(3, 1))) == rational(1, 1));
	assert(simplify(m, term(tag::DivT, rational(-5, 1), rational(3, 1))) == rational(-1, 1));
	assert(simplify(m, term(tag::DivT, rational(5, 1), rational(-3, 1))) == rational(-1, 1));
	assert(simplify(m, term(tag::DivT, rational(-5, 1), rational(-3, 1))) == rational(1, 1));

	assert(simplify(m, term(tag::RemT, integer(5), integer(3))) == integer(5 % 3));
	assert(simplify(m, term(tag::RemT, integer(-5), integer(3))) == integer(-5 % 3));
	assert(simplify(m, term(tag::RemT, integer(5), integer(-3))) == integer(5 % -3));
	assert(simplify(m, term(tag::RemT, integer(-5), integer(-3))) == integer(-5 % -3));
	assert(simplify(m, term(tag::RemT, integer(5), integer(3))) == integer(2));
	assert(simplify(m, term(tag::RemT, integer(-5), integer(3))) == integer(-2));
	assert(simplify(m, term(tag::RemT, integer(5), integer(-3))) == integer(2));
	assert(simplify(m, term(tag::RemT, integer(-5), integer(-3))) == integer(-2));
	assert(simplify(m, term(tag::RemT, rational(5, 1), rational(3, 1))) == rational(2, 1));
	assert(simplify(m, term(tag::RemT, rational(-5, 1), rational(3, 1))) == rational(-2, 1));
	assert(simplify(m, term(tag::RemT, rational(5, 1), rational(-3, 1))) == rational(2, 1));
	assert(simplify(m, term(tag::RemT, rational(-5, 1), rational(-3, 1))) == rational(-2, 1));

	assert(simplify(m, term(tag::DivE, integer(7), integer(3))) == integer(2));
	assert(simplify(m, term(tag::DivE, integer(-7), integer(3))) == integer(-3));
	assert(simplify(m, term(tag::DivE, integer(7), integer(-3))) == integer(-2));
	assert(simplify(m, term(tag::DivE, integer(-7), integer(-3))) == integer(3));
	assert(simplify(m, term(tag::DivE, rational(7, 1), rational(3, 1))) == rational(2, 1));
	assert(simplify(m, term(tag::DivE, rational(-7, 1), rational(3, 1))) == rational(-3, 1));
	assert(simplify(m, term(tag::DivE, rational(7, 1), rational(-3, 1))) == rational(-2, 1));
	assert(simplify(m, term(tag::DivE, rational(-7, 1), rational(-3, 1))) == rational(3, 1));

	assert(simplify(m, term(tag::RemE, integer(7), integer(3))) == integer(1));
	assert(simplify(m, term(tag::RemE, integer(-7), integer(3))) == integer(2));
	assert(simplify(m, term(tag::RemE, integer(7), integer(-3))) == integer(1));
	assert(simplify(m, term(tag::RemE, integer(-7), integer(-3))) == integer(2));
	assert(simplify(m, term(tag::RemE, rational(7, 1), rational(3, 1))) == rational(1, 1));
	assert(simplify(m, term(tag::RemE, rational(-7, 1), rational(3, 1))) == rational(2, 1));
	assert(simplify(m, term(tag::RemE, rational(7, 1), rational(-3, 1))) == rational(1, 1));
	assert(simplify(m, term(tag::RemE, rational(-7, 1), rational(-3, 1))) == rational(2, 1));

	assert(simplify(m, term(tag::Ceil, integer(0))) == integer(0));
	assert(simplify(m, term(tag::Ceil, rational(0, 1))) == rational(0, 1));
	assert(simplify(m, term(tag::Ceil, rational(1, 10))) == rational(1, 1));
	assert(simplify(m, term(tag::Ceil, rational(5, 10))) == rational(1, 1));
	assert(simplify(m, term(tag::Ceil, rational(9, 10))) == rational(1, 1));
	assert(simplify(m, term(tag::Ceil, rational(-1, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Ceil, rational(-5, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Ceil, rational(-9, 10))) == rational(0, 1));

	assert(simplify(m, term(tag::Floor, integer(0))) == integer(0));
	assert(simplify(m, term(tag::Floor, rational(0, 1))) == rational(0, 1));
	assert(simplify(m, term(tag::Floor, rational(1, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Floor, rational(5, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Floor, rational(9, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Floor, rational(-1, 10))) == rational(-1, 1));
	assert(simplify(m, term(tag::Floor, rational(-5, 10))) == rational(-1, 1));
	assert(simplify(m, term(tag::Floor, rational(-9, 10))) == rational(-1, 1));

	assert(simplify(m, term(tag::Trunc, integer(0))) == integer(0));
	assert(simplify(m, term(tag::Trunc, rational(0, 1))) == rational(0, 1));
	assert(simplify(m, term(tag::Trunc, rational(1, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Trunc, rational(5, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Trunc, rational(9, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Trunc, rational(-1, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Trunc, rational(-5, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Trunc, rational(-9, 10))) == rational(0, 1));

	assert(simplify(m, term(tag::Round, integer(0))) == integer(0));
	assert(simplify(m, term(tag::Round, rational(0, 1))) == rational(0, 1));
	assert(simplify(m, term(tag::Round, rational(1, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Round, rational(5, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Round, rational(9, 10))) == rational(1, 1));
	assert(simplify(m, term(tag::Round, rational(-1, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Round, rational(-5, 10))) == rational(0, 1));
	assert(simplify(m, term(tag::Round, rational(-9, 10))) == rational(-1, 1));
	assert(simplify(m, term(tag::Round, rational(15, 10))) == rational(2, 1));
	assert(simplify(m, term(tag::Round, rational(25, 10))) == rational(2, 1));
	assert(simplify(m, term(tag::Round, rational(35, 10))) == rational(4, 1));
	assert(simplify(m, term(tag::Round, rational(45, 10))) == rational(4, 1));

	assert(simplify(m, term(tag::Add, rational(1, 7), rational(2, 7))) == rational(3, 7));
	assert(simplify(m, term(tag::Add, real(1, 7), real(2, 7))) == real(3, 7));

	assert(simplify(m, term(tag::IsInteger, rational(5, 5))) == tag::True);
	assert(simplify(m, term(tag::IsInteger, rational(5, 10))) == tag::False);

	assert(simplify(m, term(tag::ToInteger, integer(0))) == integer(0));
	assert(simplify(m, term(tag::ToInteger, rational(0, 1))) == integer(0));
	assert(simplify(m, term(tag::ToInteger, rational(1, 10))) == integer(0));
	assert(simplify(m, term(tag::ToInteger, rational(5, 10))) == integer(0));
	assert(simplify(m, term(tag::ToInteger, rational(9, 10))) == integer(0));
	assert(simplify(m, term(tag::ToInteger, rational(-1, 10))) == integer(-1));
	assert(simplify(m, term(tag::ToInteger, rational(-5, 10))) == integer(-1));
	assert(simplify(m, term(tag::ToInteger, rational(-9, 10))) == integer(-1));

	assert(simplify(m, term(tag::ToRational, integer(7))) == rational(7, 1));
	assert(simplify(m, term(tag::ToRational, rational(7, 1))) == rational(7, 1));

	assert(simplify(m, term(tag::ToReal, integer(7))) == real(7, 1));
	assert(simplify(m, term(tag::ToReal, rational(7, 1))) == real(7, 1));

	assert(simplify(m, term(tag::Ceil, real(5, 1))) == real(5, 1));
	assert(simplify(m, term(tag::IsInteger, real(5, 1))) == tag::True);
	assert(simplify(m, term(tag::IsInteger, gensym(kind::Integer))) == tag::True);

	a = gensym(kind::Integer);
	assert(simplify(m, term(tag::Neg, a)) == term(tag::Neg, a));
	assert(simplify(m, term(tag::Ceil, a)) == term(tag::Ceil, a));
	assert(simplify(m, term(tag::Floor, a)) == term(tag::Floor, a));
	assert(simplify(m, term(tag::Trunc, a)) == term(tag::Trunc, a));
	assert(simplify(m, term(tag::Round, a)) == term(tag::Round, a));
	assert(simplify(m, term(tag::IsInteger, a)) == tag::True);
	assert(simplify(m, term(tag::IsRational, a)) == tag::True);
	assert(simplify(m, term(tag::ToInteger, a)) == a);
	assert(simplify(m, term(tag::ToRational, a)) == term(tag::ToRational, a));
	assert(simplify(m, term(tag::ToReal, a)) == term(tag::ToReal, a));

	a = gensym(kind::Rational);
	assert(simplify(m, term(tag::Neg, a)) == term(tag::Neg, a));
	assert(simplify(m, term(tag::Ceil, a)) == term(tag::Ceil, a));
	assert(simplify(m, term(tag::Floor, a)) == term(tag::Floor, a));
	assert(simplify(m, term(tag::Trunc, a)) == term(tag::Trunc, a));
	assert(simplify(m, term(tag::Round, a)) == term(tag::Round, a));
	assert(simplify(m, term(tag::IsInteger, a)) == term(tag::IsInteger, a));
	assert(simplify(m, term(tag::IsRational, a)) == tag::True);
	assert(simplify(m, term(tag::ToInteger, a)) == term(tag::ToInteger, a));
	assert(simplify(m, term(tag::ToRational, a)) == a);
	assert(simplify(m, term(tag::ToReal, a)) == term(tag::ToReal, a));

	a = gensym(kind::Real);
	assert(simplify(m, term(tag::Neg, a)) == term(tag::Neg, a));
	assert(simplify(m, term(tag::Ceil, a)) == term(tag::Ceil, a));
	assert(simplify(m, term(tag::Floor, a)) == term(tag::Floor, a));
	assert(simplify(m, term(tag::Trunc, a)) == term(tag::Trunc, a));
	assert(simplify(m, term(tag::Round, a)) == term(tag::Round, a));
	assert(simplify(m, term(tag::IsInteger, a)) == term(tag::IsInteger, a));
	assert(simplify(m, term(tag::IsRational, a)) == term(tag::IsRational, a));
	assert(simplify(m, term(tag::ToInteger, a)) == term(tag::ToInteger, a));
	assert(simplify(m, term(tag::ToRational, a)) == term(tag::ToRational, a));
	assert(simplify(m, term(tag::ToReal, a)) == a);

	// Unification.
	testMatch();
	testUnify();

	// Free variables.
	a = gensym(kind::Individual);
	x = var(600, kind::Individual);
	y = var(601, kind::Individual);

	auto fv = freeVars(a);
	assert(fv.size() == 0);

	fv = freeVars(x);
	assert(fv.size() == 1);
	assert(*fv.begin() == x);

	fv = freeVars(term(tag::Eq, x, x));
	assert(fv.size() == 1);
	assert(*fv.begin() == x);

	fv = freeVars(term(tag::Eq, x, y));
	assert(fv.size() == 2);

	fv = freeVars(term(tag::Eq, a, a));
	assert(fv.size() == 0);

	fv = freeVars(term(tag::All, term(tag::Eq, x, y), x));
	assert(fv.size() == 1);
	assert(*fv.begin() == y);

	// Cartesian product.
	testCartProduct();

	// flattenTerm.
	vec<term> r{integer(1), integer(2), integer(3), integer(4), integer(5)};
	assert(flatten(tag::Add, term(tag::Add, integer(1), integer(2), term(tag::Add, integer(3), integer(4), integer(5)))) == r);

	// Vectors.
	assert(sum(vec<size_t>{1, 2, 3}) == 6);

	// Parsing real numbers.
	testReal("123", 123, 1);
	testReal("123", 123, 1);
	testReal("-123", -123, 1);
	testReal("-123.", -123, 1);
	testReal("-.123", -123, 1000);
	testReal("123e3", 123000, 1);
	testReal("123e+3", 123000, 1);
	testReal("123e-3", 123, 1000);
	testReal("123.456", 123456, 1000);
	testReal("123.456e3", 123456, 1);

	// Cnf.
	testCnf();

	// Subsumption.
	testSubsume();

	// Vec.
	vec<int> v1{1, 2, 3};
	for (auto& a: v1) a++;
	vec<int> v2{2, 3, 4};
	for (int i = 0; i != v1.size(); ++i) assert(v1[i] == v2[i]);

	// Dpll.
	testDpll();
	heap->check();
}
#endif
