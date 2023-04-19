#include "main.h"

string keywords[] = {
	// clang-format off
	{0, 0, 0, "?"},
	{0, 0, 0, "C"},
	{0, 0, 0, "T"},
	{0, 0, 0, "V"},
	{0, 0, 0, "ax"},
	{0, 0, 0, "bool"},
	{0, 0, 0, "break"},
	{0, 0, 0, "cc"},
	{0, 0, 0, "ceiling"},
	{0, 0, 0, "clause"},
	{0, 0, 0, "cnf"},
	{0, 0, 0, "conjecture"},
	{0, 0, 0, "continue"},
	{0, 0, 0, "cpp"},
	{0, 0, 0, "cpulimit"},
	{0, 0, 0, "cxx"},
	{0, 0, 0, "difference"},
	{0, 0, 0, "dimacs"},
	{0, 0, 0, "dimacsin"},
	{0, 0, 0, "dimacsout"},
	{0, 0, 0, "distinct"},
	{0, 0, 0, "do"},
	{0, 0, 0, "else"},
	{0, 0, 0, "false"},
	{0, 0, 0, "floor"},
	{0, 0, 0, "fof"},
	{0, 0, 0, "for"},
	{0, 0, 0, "graph"},
	{0, 0, 0, "greater"},
	{0, 0, 0, "greatereq"},
	{0, 0, 0, "h"},
	{0, 0, 0, "help"},
	{0, 0, 0, "i"},
	{0, 0, 0, "if"},
	{0, 0, 0, "in"},
	{0, 0, 0, "include"},
	{0, 0, 0, "int"},
	{0, 0, 0, "is_int"},
	{0, 0, 0, "is_rat"},
	{0, 0, 0, "less"},
	{0, 0, 0, "lesseq"},
	{0, 0, 0, "m"},
	{0, 0, 0, "map"},
	{0, 0, 0, "memory"},
	{0, 0, 0, "memorylimit"},
	{0, 0, 0, "o"},
	{0, 0, 0, "p"},
	{0, 0, 0, "product"},
	{0, 0, 0, "quotient"},
	{0, 0, 0, "quotient_e"},
	{0, 0, 0, "quotient_f"},
	{0, 0, 0, "quotient_t"},
	{0, 0, 0, "rat"},
	{0, 0, 0, "real"},
	{0, 0, 0, "remainder_e"},
	{0, 0, 0, "remainder_f"},
	{0, 0, 0, "remainder_t"},
	{0, 0, 0, "return"},
	{0, 0, 0, "round"},
	{0, 0, 0, "set"},
	{0, 0, 0, "sum"},
	{0, 0, 0, "t"},
	{0, 0, 0, "tType"},
	{0, 0, 0, "tff"},
	{0, 0, 0, "to_int"},
	{0, 0, 0, "to_rat"},
	{0, 0, 0, "to_real"},
	{0, 0, 0, "tptp"},
	{0, 0, 0, "tptpin"},
	{0, 0, 0, "tptpout"},
	{0, 0, 0, "true"},
	{0, 0, 0, "truncate"},
	{0, 0, 0, "type"},
	{0, 0, 0, "uminus"},
	{0, 0, 0, "val"},
	{0, 0, 0, "vector"},
	{0, 0, 0, "version"},
	{0, 0, 0, "void"},
	{0, 0, 0, "while"},
	// clang-format on
};

namespace {
// For data to be kept for the full duration of the process, we can avoid the time and memory overhead of keeping track of
// individual allocations. This allocator is specialized; it aligns allocations only as needed by strings.
void* alloc(size_t bytes) {
	bytes = roundUp(bytes, 4);
	static char* top;
	static char* end;
	if (end - top < bytes) {
		auto n = max(bytes, (size_t)10000);
		top = new char[n];
		end = top + n;
	}
	auto r = top;
#ifdef DEBUG
	memset(r, 0xcc, bytes);
#endif
	top += bytes;
	return r;
}

// Compare a counted string with a null terminated one.
bool eq(const char* s, size_t n, const char* z) {
	while (n--)
		if (*s++ != *z++) return 0;
	return !*z;
}

size_t slot(string** entries, size_t cap, const char* s, size_t n) {
	size_t mask = cap - 1;
	auto i = fnv(s, n) & mask;
	while (entries[i] && !eq(s, n, entries[i]->v)) i = (i + 1) & mask;
	return i;
}

size_t cap = 0x100;
size_t qty = end_s;
string** entries;

struct init {
	init() {
		static_assert(isPow2(sizeof(string)));
		assert(isPow2(cap));
		assert(qty <= cap * 3 / 4);
		entries = (string**)calloc(cap, sizeof(string*));
		for (int i = 0; i != sizeof keywords / sizeof *keywords; ++i) {
			auto s = keywords + i;
			auto n = strlen(s->v);

			// C++ allows the edge case where a string literal exactly fills an array, leaving no room for a null terminator. This
			// is sometimes useful, but would not be appropriate here, so make sure it's not the case.
			assert(n < sizeof keywords[0].v);

			// Make sure there are no duplicate keywords.
			assert(!entries[slot(entries, cap, s->v, n)]);

			// Add to hash table.
			entries[slot(entries, cap, s->v, n)] = s;
		}
	}
} _;

void expand() {
	auto cap1 = cap * 2;
	auto entries1 = (string**)calloc(cap1, sizeof(string*));
	for (size_t i = 0; i != cap; ++i) {
		auto s = entries[i];
		if (s) entries1[slot(entries1, cap1, s->v, strlen(s->v))] = s;
	}
	free(entries);
	cap = cap1;
	entries = entries1;
}
} // namespace

void clearStrings() {
	for (size_t i = 0; i != cap; ++i) {
		auto s = entries[i];
		if (!s) continue;
		s->dobj = 0;
		s->sym = 0;
	}
}

string* intern(const char* s, size_t n) {
	auto i = slot(entries, cap, s, n);

	// If we have seen this string before, return the existing string.
	if (entries[i]) return entries[i];

	// Expand the hash table if necessary.
	if (++qty > cap * 3 / 4) {
		expand();
		i = slot(entries, cap, s, n);
		assert(!entries[i]);
	}

	// Make a new string.
	auto r = (string*)alloc(offsetof(string, v) + n + 1);
	memset(r, 0, offsetof(string, v));
	memcpy(r->v, s, n);
	r->v[n] = 0;

	// Add to hash table.
	return entries[i] = r;
}
