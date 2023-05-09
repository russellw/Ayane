#include "main.h"

Ex bools[2] = {{Tag::false1}, {Tag::true1}};

// TODO: rename?
Ex* gensym(Type* ty) {
	auto a = (Ex*)malloc(offsetof(Ex, s) + sizeof(char*));
	a->tag = Tag::fn;
	a->ty = ty;
	a->s = 0;
	return a;
}

// Composite expressions
struct CompCmp {
	static bool eq(Tag tag, Ex** a, size_t n, Ex* b) {
		return tag == b->tag && n == b->n && memcmp(a, b->v, n * sizeof *a) == 0;
	}
	static bool eq(Ex* a, Ex* b) {
		return eq(a->tag, a->v, a->n, b);
	}

	static size_t hash(Tag tag, Ex** a, size_t n) {
		// TODO: hashCombine?
		return fnv(a, n * sizeof *a);
	}
	static size_t hash(Ex* a) {
		return hash(a->tag, a->v, a->n);
	}
};

static void clear(Ex** a) {
}

static Set<Tag, Ex**, Ex, CompCmp> comps;

Ex* ex(Tag tag, Ex* a, Ex* b) {
	static Ex* v[2];
	v[0] = a;
	v[1] = b;
	return comps.intern(tag, v, 2);
}

Ex* ex(Tag tag, const Vec<Ex*>& v) {
	assert(v.size());
	return comps.intern(tag, v.data, v.n);
}

Type* type(Ex* a) {
	switch (a->tag) {
	case Tag:: or:
	case Tag::all:
	case Tag::and:
	case Tag::eq:
	case Tag::eqv:
	case Tag::exists:
	case Tag::false1:
	case Tag::isInteger:
	case Tag::isRational:
	case Tag::le:
	case Tag::lt:
	case Tag::not:
	case Tag::true1:
		return &tbool;
	case Tag::add:
	case Tag::ceil:
	case Tag::div:
	case Tag::dive:
	case Tag::divf:
	case Tag::divt:
	case Tag::floor:
	case Tag::minus:
	case Tag::mul:
	case Tag::reme:
	case Tag::remf:
	case Tag::remt:
	case Tag::round:
	case Tag::sub:
	case Tag::trunc:
		return type(at(a, 0));
	case Tag::call:
		a = at(a, 0);
		assert(a->tag == Tag::fn);
		return at(a->ty, 0);
	case Tag::distinctObj:
		return &tindividual;
	case Tag::fn:
	case Tag::var:
		return a->ty;
	case Tag::integer:
	case Tag::toInteger:
		return &tinteger;
	case Tag::rational:
	case Tag::toRational:
		return &trational;
	case Tag::real:
	case Tag::toReal:
		return &treal;
	}
	unreachable;
}

Type* ftype(Type* rty, Ex** first, Ex** last) {
	if (first == last) return rty;
	Vec<Type*> v(1, rty);
	// TODO: add in one op
	for (auto i = first; i < last; ++i) v.add(type(*i));
	return type(v);
}

int cmp(Ex* a, Ex* b) {
	assert(a->tag == b->tag);
	if (a == b) return 0;
	switch (a->tag) {
	case Tag::integer:
		return mpz_cmp(a->mpz, b->mpz);
	case Tag::rational:
	case Tag::real:
		return mpq_cmp(a->mpq, b->mpq);
	}
	unreachable;
}
