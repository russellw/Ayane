#include "main.h"

Str keywords[] = {
	// clang-format off
	{0, 0, "ax"},
	{0, 0, "bool"},
	{0, 0, "ceiling"},
	{0, 0, "cnf"},
	{0, 0, "conjecture"},
	{0, 0, "difference"},
	{0, 0, "distinct"},
	{0, 0, "false"},
	{0, 0, "floor"},
	{0, 0, "fof"},
	{0, 0, "greater"},
	{0, 0, "greatereq"},
	{0, 0, "i"},
	{0, 0, "include"},
	{0, 0, "int"},
	{0, 0, "is_int"},
	{0, 0, "is_rat"},
	{0, 0, "less"},
	{0, 0, "lesseq"},
	{0, 0, "o"},
	{0, 0, "p"},
	{0, 0, "product"},
	{0, 0, "quotient"},
	{0, 0, "quotient_e"},
	{0, 0, "quotient_f"},
	{0, 0, "quotient_t"},
	{0, 0, "rat"},
	{0, 0, "real"},
	{0, 0, "remainder_e"},
	{0, 0, "remainder_f"},
	{0, 0, "remainder_t"},
	{0, 0, "round"},
	{0, 0, "sum"},
	{0, 0, "tType"},
	{0, 0, "tff"},
	{0, 0, "to_int"},
	{0, 0, "to_rat"},
	{0, 0, "to_real"},
	{0, 0, "true"},
	{0, 0, "truncate"},
	{0, 0, "type"},
	{0, 0, "uminus"},
	// clang-format on
};

// TODO: factor to  set
namespace {
// Compare a counted string with a null terminated one
bool eq(const char* s, size_t n, const char* z) {
	while (n--)
		if (*s++ != *z++) return 0;
	return !*z;
}

size_t slot(Str** entries, size_t cap, const char* s, size_t n) {
	size_t mask = cap - 1;
	auto i = fnv(s, n) & mask;
	while (entries[i] && !eq(s, n, entries[i]->v)) i = (i + 1) & mask;
	return i;
}

size_t cap = 0x100;
size_t qty = nkeywords;
Str** entries;

struct init {
	init() {
		static_assert(isPow2(sizeof(Str)));
		assert(isPow2(cap));
		assert(qty <= cap * 3 / 4);
		entries = (Str**)calloc(cap, sizeof(Str*));
		for (int i = 0; i < sizeof keywords / sizeof *keywords; ++i) {
			auto s = keywords + i;
			auto n = strlen(s->v);

			// C++ allows the edge case where a string literal exactly fills an array, leaving no room for a null terminator. This
			// is sometimes useful, but would not be appropriate here, so make sure it's not the case.
			assert(n < sizeof keywords[0].v);

			// Make sure there are no duplicate keywords
			assert(!entries[slot(entries, cap, s->v, n)]);

			// Add to hash table
			entries[slot(entries, cap, s->v, n)] = s;
		}
	}
} _;

void expand() {
	auto cap1 = cap * 2;
	auto entries1 = (Str**)calloc(cap1, sizeof(Str*));
	for (size_t i = 0; i < cap; ++i) {
		auto s = entries[i];
		if (s) entries1[slot(entries1, cap1, s->v, strlen(s->v))] = s;
	}
	free(entries);
	cap = cap1;
	entries = entries1;
}
} // namespace

Str* intern(const char* s, size_t n) {
	auto i = slot(entries, cap, s, n);

	// If we have seen this string before, return the existing string
	if (entries[i]) return entries[i];

	// Expand the hash table if necessary
	if (++qty > cap * 3 / 4) {
		expand();
		i = slot(entries, cap, s, n);
		assert(!entries[i]);
	}

	// Make a new string
	auto r = (Str*)malloc(offsetof(Str, v) + n + 1);
	memset(r, 0, offsetof(Str, v));
	memcpy(r->v, s, n);
	r->v[n] = 0;

	// Add to hash table
	return entries[i] = r;
}

Type* type(Str* s) {
	if (s->ty) return s->ty;
	auto ty = (TypeName*)malloc(sizeof(TypeName));
	ty->tag = False;
	ty->n = 0;
	ty->s = s->v;
	return s->ty = ty;
}
