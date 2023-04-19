#include "main.h"

char typeMem[(1 << typeBits) * 8];

namespace {
const char* kindNames[] = {
#define _(x) #x,
#include "kinds.h"
};

size_t alloc(size_t n) {
	// TODO: optimize
	n = roundUp(n, 8);

	// Reserve the first few words of memory for unboxed atomic types.
	static size_t top = unboxedTypes;

	// Check overflow.
	const size_t end = 1 << typeBits;
	if (end - top < n / 8) err("Too many types");

	// Bump allocation.
	auto o = top;
#ifdef DEBUG
	memset(typePtr(o), 0xcc, n);
#endif
	top += n / 8;
	return o;
}
} // namespace

type::type(string* s) {
	auto o = s->ty;
	if (!o) {
		o = alloc(sizeof(Type));
		auto p = typePtr(o);
		p->k = kind::Sym;
		p->n = 0;
		p->s = s->v;
		s->ty = o;
	}
	offset = o;
}

namespace {
// Compare a separately counted array with one that contains its own count. This is analogous to the problem of interning strings,
// where a separately counted string must be compared with a null terminated one, so the analogous variable names have been left as
// they are.
bool eq(kind k, const type* s, size_t n, const Type* z) {
	if (k != z->k) return 0;
	if (n != z->n) return 0;
	return !memcmp(s, z->v, n * sizeof *s);
}

// The hash table capacity must be strictly greater than the number of types we can have, because the capacity factor needs to be
// less than 100%.
const size_t cap = 1 << (typeBits + 1);
uint32_t entries[cap];

struct init {
	init() {
		assert(isPow2(cap));
		for (size_t i = 1; i != unboxedTypes; ++i) typePtr(i * offsetof(Type, v) / 8)->k = (kind)i;
	}
} _;

size_t slot(kind k, const type* s, size_t n) {
	size_t mask = cap - 1;
	auto i = fnv(s, n * sizeof *s) & mask;
	while (entries[i] && !eq(k, s, n, typePtr(entries[i]))) i = (i + 1) & mask;
	return i;
}

size_t intern(kind k, const type* s, size_t n) {
	auto i = slot(k, s, n);

	// If we have seen this before, return the existing object.
	if (entries[i]) return entries[i];

	// Make a new object.
	auto o = alloc(offsetof(Type, v) + n * sizeof *s);
	auto p = typePtr(o);
	p->k = k;
	p->n = n;
	memcpy(p->v, s, n * sizeof *s);

	// Add to hash table.
	return entries[i] = o;
}
} // namespace

type::type(kind k, type a) {
	offset = intern(k, &a, 1);
}

type::type(kind k, type a, type b) {
	const size_t n = 2;
	type v[n];
	v[0] = a;
	v[1] = b;
	offset = intern(k, v, n);
}

type::type(kind k, const vec<type>& v) {
	offset = intern(k, v.data(), v.size());
}

void assign(type& t, type u) {
	if (t == u) return;
	if (t == kind::Unknown) {
		t = u;
		return;
	}
	if (u == kind::Unknown) return;
	err("Type mismatch");
}

void print(kind k) {
	print(kindNames[(size_t)k]);
}

void print(type ty) {
	if (kind(ty) == kind::Sym) {
		auto p = typePtr(ty.offset);
		print(p->s);
		return;
	}
	print(kind(ty));
	if (!ty.size()) return;
	putchar('(');
	joining;
	for (size_t i = 0; i != ty.size(); ++i) {
		join(", ");
		print(ty[i]);
	}
	putchar(')');
}
