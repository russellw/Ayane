#include "main.h"

// Atoms.
Heap<8>* atoms;
uint32_t tatoms;

namespace {
struct init {
	init() {
		atoms = Heap<8>::make();
		auto n = (int)tag::end;
		tatoms = atoms->alloc(n * offsetof(atom, s));
		for (int i = 0; i != n; ++i) {
			auto p = (atom*)atoms->ptr(tatoms + i * offsetof(atom, s) / 8);
			p->t = (tag)i;
		}
	}
} _;

map<type, vec<term>> boxedVars;
} // namespace

term var(size_t i, type ty) {
	auto unboxed = 1 << term::idxBits;
	if (i < unboxed) {
		term a;
		a.raw = term::t_var | i << typeBits | ty.offset;
		return a;
	}
	auto& v = boxedVars.gadd(ty);
	while (v.size() <= i - unboxed) {
		auto o = atoms->alloc(offsetof(atom, idx) + 4);
		auto p = (atom*)atoms->ptr(o);
		p->t = tag::Var;
		p->ty = ty;
		p->idx = unboxed + v.size();
		term a;
		a.raw = term::t_var | term::t_boxed | o;
		v.push_back(a);
	}
	return v[i - unboxed];
}

term distinctObj(string* s) {
	term a;
	if (s->dobj) {
		a.raw = s->dobj;
		return a;
	}
	auto o = atoms->alloc(offsetof(atom, s) + sizeof s);
	auto p = (atom*)atoms->ptr(o);
	p->t = tag::DistinctObj;
	p->s = s->v;
	a.raw = s->dobj = o;
	return a;
}

term::term(string* s, type ty) {
	if (s->sym) {
		auto p = (atom*)atoms->ptr(s->sym);
		assert(p->t == tag::Fn);
		assert(p->s == s->v);
		assign(p->ty, ty);
	} else {
		s->sym = atoms->alloc(offsetof(atom, s) + sizeof s);
		auto p = (atom*)atoms->ptr(s->sym);
		p->t = tag::Fn;
		p->s = s->v;
		p->ty = ty;
	}
	raw = s->sym;
}

term gensym(type ty) {
	auto o = atoms->alloc(offsetof(atom, s) + sizeof(char*));
	auto p = (atom*)atoms->ptr(o);
	p->t = tag::Fn;
	p->s = 0;
	p->ty = ty;
	term a;
	a.raw = o;
	return a;
}

// Compounds.
Heap<>* compounds;

namespace comps {
bool eq(const term* s, size_t n, const compound* z) {
	if (n != z->n) return 0;
	return !memcmp(s, z->v, n * sizeof *s);
}

size_t slot(uint32_t* entries, size_t cap, const term* s, size_t n) {
	size_t mask = cap - 1;
	auto i = fnv(s, n * sizeof *s) & mask;
	while (entries[i] && !eq(s, n, (compound*)compounds->ptr(entries[i]))) i = (i + 1) & mask;
	return i;
}

size_t cap = 0x100;
size_t qty;
uint32_t* entries;

struct init {
	init() {
		compounds = Heap<>::make();
		assert(isPow2(cap));
		entries = (uint32_t*)compounds->ptr(compounds->calloc(cap * 4));
	}
} _;

void expand() {
	auto cap1 = cap * 2;
	auto entries1 = (uint32_t*)compounds->ptr(compounds->calloc(cap1 * 4));
	for (size_t i = 0; i != cap; ++i) {
		auto o = entries[i];
		if (!o) continue;
		auto s = (compound*)compounds->ptr(o);
		entries1[slot(entries1, cap1, s->v, s->n)] = o;
	}
	compounds->free(compounds->offset(entries), cap * 4);
	cap = cap1;
	entries = entries1;
}

size_t intern(const term* s, size_t n) {
	incStat("term");
	auto i = slot(entries, cap, s, n);

	// If we have seen this before, return the existing object.
	if (entries[i]) return entries[i];

	// Expand the hash table if necessary.
	if (++qty > cap * 3 / 4) {
		expand();
		i = slot(entries, cap, s, n);
		assert(!entries[i]);
	}

	// Make a new object.
	incStat("term alloc");
	incStat("term alloc bytes", offsetof(compound, v) + n * sizeof *s);
	auto o = compounds->alloc(offsetof(compound, v) + n * sizeof *s);
	if (o & term::t_compound) err("compound term: Out of memory");
	auto p = (compound*)compounds->ptr(o);
	p->n = n;
	memcpy(p->v, s, n * sizeof *s);

	// Add to hash table.
	return entries[i] = o;
}
} // namespace comps

term::term(term a, term b) {
	const int n = 2;
	term v[n];
	v[0] = a;
	v[1] = b;
	raw = t_compound | comps::intern(v, n);
}

term::term(term a, term b, term c) {
	const int n = 3;
	term v[n];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	raw = t_compound | comps::intern(v, n);
}

term::term(term a, term b, term c, term d) {
	const int n = 4;
	term v[n];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = d;
	raw = t_compound | comps::intern(v, n);
}

term::term(term a, term b, term c, term d, term e) {
	const int n = 5;
	term v[n];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = d;
	v[4] = e;
	raw = t_compound | comps::intern(v, n);
}

term::term(const vec<term>& v) {
	assert(v.size());
	if (v.size() == 1) *this = v[0];
	else
		raw = t_compound | comps::intern(v.data(), v.size());
}

term::operator type() const {
	switch (tag(*this)) {
	case tag::Add:
	case tag::Ceil:
	case tag::Div:
	case tag::DivE:
	case tag::DivF:
	case tag::DivT:
	case tag::Floor:
	case tag::Mul:
	case tag::Neg:
	case tag::RemE:
	case tag::RemF:
	case tag::RemT:
	case tag::Round:
	case tag::Sub:
	case tag::Trunc:
		return type((*this)[1]);
	case tag::All:
	case tag::And:
	case tag::Eq:
	case tag::Eqv:
	case tag::Exists:
	case tag::False:
	case tag::IsInteger:
	case tag::IsRational:
	case tag::Le:
	case tag::Lt:
	case tag::Not:
	case tag::Or:
	case tag::True:
		return kind::Bool;
	case tag::DistinctObj:
		return kind::Individual;
	case tag::Fn:
	{
		// TODO: optimize
		auto ty = getAtom()->ty;
		if (kind(ty) == kind::Fn) return ty[0];
		return ty;
	}
	case tag::Integer:
	case tag::ToInteger:
		return kind::Integer;
	case tag::Rational:
	case tag::ToRational:
		return kind::Rational;
	case tag::ToReal:
		return kind::Real;
	case tag::Var:
		if (raw & t_boxed) return getAtom()->ty;
		return type(raw & (1 << typeBits) - 1);
	}
	unreachable;
}

type ftype(type rty, const term* first, const term* last) {
	if (first == last) return rty;
	vec<type> v(1, rty);
	for (auto i = first; i != last; ++i) v.push_back(type(*i));
	return type(kind::Fn, v);
}

type ftype(type rty, const set<term>& args) {
	if (args.size()) return rty;
	vec<type> v(1, rty);
	for (auto a: args) v.push_back(type(a));
	return type(kind::Fn, v);
}

size_t term::size() const {
	if (!(raw & t_compound)) return 1;
	return getCompound()->n;
}

term term::operator[](size_t i) const {
	assert(i < size());
	if (!(raw & t_compound)) return *this;
	return getCompound()->v[i];
}

// TODO: eliminate this?
int cmp(term a, term b) {
	// Fast test for equality.
	if (a == b) return 0;

	// If the tags differ, just sort in tag order; not meaningful, but it doesn't have to be meaningful, just consistent.
	if (tag(a) != tag(b)) return (int)tag(a) - (int)tag(b);

	// Numbers sort in numerical order.
	switch (tag(a)) {
	case tag::Integer:
		return mpz_cmp(a.mpz(), b.mpz());
	case tag::Rational:
		return mpq_cmp(a.mpq(), b.mpq());
	}

	// Compound terms sort in lexicographic order.
	auto an = a.size();
	auto bn = b.size();
	auto n = min(an, bn);
	for (size_t i = 1; i != n; ++i) {
		auto c = cmp(a[i], b[i]);
		if (c) return c;
	}
	if (an - bn) return an - bn;

	// They are different terms with the same tags and no different components, so they must be different atoms; just do a straight
	// binary comparison.
	return memcmp(&a, &b, sizeof a);
}

void print(tag t) {
	static const char* tagNames[] = {
#define _(x) #x,
#include "tags.h"
	};
	print(tagNames[(int)t]);
}

void print(term a) {
	switch (tag(a)) {
	case tag::Fn:
	{
		auto p = a.getAtom();
		auto s = p->s;
		if (s) print(s);
		else
			printf("_%p", p);
		break;
	}
	case tag::Integer:
		mpz_out_str(stdout, 10, a.mpz());
		return;
	case tag::Rational:
		mpq_out_str(stdout, 10, a.mpq());
		return;
	case tag::Var:
		printf("%%%zu", a.varIdx());
		return;
	default:
		print(tag(a));
		break;
	}
	if (a.size() == 1) return;
	putchar('(');
	joining;
	for (auto b: a) {
		join(", ");
		print(b);
	}
	putchar(')');
}
