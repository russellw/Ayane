#include "main.h"

Str keywords[] = {
	// clang-format off
	{0, 0, "Bool"},
	{0, 0, "Int"},
	{0, 0, "Real"},
	{0, 0, "String"},
	{0, 0, "and"},
	{0, 0, "assert"},
	{0, 0, "ax"},
	{0, 0, "bool"},
	{0, 0, "bvand"},
	{0, 0, "bvnot"},
	{0, 0, "bvor"},
	{0, 0, "bvxor"},
	{0, 0, "ceiling"},
	{0, 0, "check-sat"},
	{0, 0, "cnf"},
	{0, 0, "conjecture"},
	{0, 0, "declare-datatypes"},
	{0, 0, "declare-fun"},
	{0, 0, "declare-sort"},
	{0, 0, "define-fun"},
	{0, 0, "define-sort"},
	{0, 0, "difference"},
	{0, 0, "distinct"},
	{0, 0, "div"},
	{0, 0, "exists"},
	{0, 0, "false"},
	{0, 0, "floor"},
	{0, 0, "fof"},
	{0, 0, "forall"},
	{0, 0, "greater"},
	{0, 0, "greatereq"},
	{0, 0, "i"},
	{0, 0, "include"},
	{0, 0, "int"},
	{0, 0, "is_int"},
	{0, 0, "is_rat"},
	{0, 0, "ite"},
	{0, 0, "less"},
	{0, 0, "lesseq"},
	{0, 0, "let"},
	{0, 0, "mod"},
	{0, 0, "not"},
	{0, 0, "o"},
	{0, 0, "or"},
	{0, 0, "p"},
	{0, 0, "product"},
	{0, 0, "push"},
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
	{0, 0, "set-info"},
	{0, 0, "set-logic"},
	{0, 0, "smt2"},
	{0, 0, "sum"},
	{0, 0, "tType"},
	{0, 0, "tcf"},
	{0, 0, "tff"},
	{0, 0, "thf"},
	{0, 0, "to_int"},
	{0, 0, "to_rat"},
	{0, 0, "to_real"},
	{0, 0, "true"},
	{0, 0, "truncate"},
	{0, 0, "type"},
	{0, 0, "uminus"},
	{0, 0, "xor"},
	{0, 0, "!"},
	{0, 0, "*"},
	{0, 0, "+"},
	{0, 0, "-"},
	{0, 0, "/"},
	{0, 0, "<"},
	{0, 0, "<="},
	{0, 0, "="},
	{0, 0, "=>"},
	{0, 0, ">"},
	{0, 0, ">="},
	// clang-format on
};

namespace {
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

#include "set.h"
const int initCap = 0x100;
Set<int, char*, Str, initCap> strs;

struct Init {
	Init() {
		const int qty = sizeof keywords / sizeof *keywords;
		static_assert(qty <= initCap * 3 / 4);
		strs.qty = qty;
		for (auto a = keywords; a < keywords + qty; ++a) {
			// C++ allows the edge case where a string literal exactly fills an array, leaving no room for a null terminator. This
			// is sometimes useful, but would not be appropriate here, so make sure it's not the case.
			assert(strlen(a->v) < sizeof a->v);

			// Where in the hash table does this keyword belong?
			auto i = strs.slot(strs.entries, strs.cap, a);

			// Make sure there are no duplicate keywords
			assert(!strs.entries[i]);

			// Add to hash table
			strs.entries[i] = a;
		}
	}
} init;
} // namespace

Str* intern(char* s, size_t n) {
	return strs.intern(0, s, n);
}
