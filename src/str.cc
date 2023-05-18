#include "main.h"

Str keywords[] = {
	// clang-format off
	{0, 0, "ax"},
	{0, 0, "ite"},
	{0, 0, "bool"},
	{0, 0, "thf"},
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
	{0, 0, "tcf"},
	{0, 0, "uminus"},
	// clang-format on
};

namespace {
// Compare a counted string with a null terminated one
bool eq(const char* s, size_t n, const char* z) {
	while (n--)
		if (*s++ != *z++) return 0;
	return !*z;
}

bool eq(int tag, char* a, size_t n, Str* b) {
	auto z = b->v;
	while (n--)
		if (*a++ != *z++) return 0;
	return !*z;
}
bool eq(Str* a, Str* b) {
	return strcmp(a->v, b->v) == 0;
}

size_t hash(int tag, char* a, size_t n) {
	return fnv(a, n);
}
size_t hash(Str* a) {
	return fnv(a->v);
}

void clear(char* a) {
}

Str* make(int tag, char* v, size_t n) {
	auto a = (Str*)ialloc(offsetof(Str, v) + n + 1);
	a->fn = 0;
	a->ty = 0;
	memcpy(a->v, v, n);
	a->v[n] = 0;
	return a;
}

Set<int, char*, Str, 0x100> strs;

struct Init {
	Init() {
		for (int i = 0; i < sizeof keywords / sizeof *keywords; ++i) {
			auto s = keywords + i;
			auto n = strlen(s->v);

			// C++ allows the edge case where a string literal exactly fills an array, leaving no room for a null terminator. This
			// is sometimes useful, but would not be appropriate here, so make sure it's not the case.
			assert(n < sizeof keywords->v);

			// Add to hash table
			auto r = strs.intern(0, s->v, n);

			// Make sure there are no duplicate keywords
			assert(r == s);
		}
	}
} init;
} // namespace

Str* intern(const char* s, size_t n) {
	return strs.intern(0, s, n);
}
