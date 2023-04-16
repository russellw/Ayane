enum class tag
{
#define _(x) x,
#include "tags.h"
	end
};

struct atom;
struct compound;

struct atom {
	tag t;
	type ty;
	union {
		const char* s;
		uint32_t idx;
		mpz_t mpz;
		mpq_t mpq;
	};
};

extern Heap<8>* atoms;
// TODO: can this be made a compile time constant?
extern uint32_t tatoms;

// TODO: reorder definitions
struct term {
	enum
	{
		t_compound = 1u << 31,
		t_var = 1 << 30,
		t_boxed = 1 << 29,
		idxBits = 29 - typeBits,
	};

	// TODO: is this more efficient as unsigned or int?
	uint32_t raw;

	explicit term(): raw(0) {
	}

	bool isVar() const {
		return (raw & (t_compound | t_var)) == t_var;
	}

	size_t varIdx() const {
		assert(isVar());
		if (raw & t_boxed) return getAtom()->idx;
		return raw >> typeBits & (1 << idxBits) - 1;
	}

	// Atoms are stored in the atom heap, interned to save memory and to speed up operations like equality testing.
	term(tag t): raw(tatoms + (int)t * offsetof(atom, s) / 8) {
	}

	explicit term(mpz_t val);
	explicit term(mpq_t val);

	// Wrapping a symbol in a term is a common operation. Specifying the type at the same time is less so, but still common enough
	// for this constructor to be useful.
	explicit term(string* s, type ty);

	size_t getAtomOffset() const;
	atom* getAtom() const;
	mpz_t& mpz() const;
	mpq_t& mpq() const;

	// Compound terms contain other terms, but maintain value semantics. Like atoms, they are interned.
	explicit term(term a, term b);
	explicit term(term a, term b, term c);
	explicit term(term a, term b, term c, term d);
	explicit term(term a, term b, term c, term d, term e);
	explicit term(const vec<term>& v);

	const compound* getCompound() const;

	explicit operator tag() const;

	// Type check is slightly slower.
	explicit operator type() const;

	// Compound terms can be treated in some ways like containers, but since they have value semantics, are strictly read-only. Also
	// when we iterate through a term, we normally want to iterate through the arguments but not the operator. Therefore, unlike in
	// a container, begin() skips over element zero, and the difference between begin() and end() is size()-1.
	size_t size() const;

	const term* begin() const;
	const term* end() const;

	term operator[](size_t i) const;
};

inline term tbool(bool b) {
	return b ? tag::True : tag::False;
}

// Variables are unboxed, with packed bits for index number and type.
term var(size_t i, type ty);

// Because terms are interned, equality can simply compare words.
inline bool operator==(term a, term b) {
	return !memcmp(&a, &b, sizeof b);
}

inline bool operator!=(term a, term b) {
	return !(a == b);
}

term gensym(type ty);
term distinctObj(string* s);

inline mpz_t& term::mpz() const {
	return getAtom()->mpz;
}

inline mpq_t& term::mpq() const {
	return getAtom()->mpq;
}

// Compounds.
struct compound {
	uint32_t n;
	term v[0];
};

extern Heap<>* compounds;

inline const compound* term::getCompound() const {
	assert(raw & t_compound);
	return (compound*)compounds->ptr(raw & ~t_compound);
}

inline size_t term::getAtomOffset() const {
	size_t o = raw;
	if (o & t_compound) {
		o = getCompound()->v[0].raw;
		assert(!(o & (t_compound | t_var | t_boxed)));
	}
	return o & t_boxed - 1;
}

inline atom* term::getAtom() const {
	return (atom*)atoms->ptr(getAtomOffset());
}

type ftype(type rty, const term* first, const term* last);
type ftype(type rty, const set<term>& args);

// Tag.
inline term::operator tag() const {
	if (isVar()) return tag::Var;
	return getAtom()->t;
}

inline const term* term::begin() const {
	if (!(raw & t_compound)) return 0;
	auto p = getCompound();
	assert(p->n);
	return p->v + 1;
}

inline const term* term::end() const {
	if (!(raw & t_compound)) return 0;
	auto p = getCompound();
	assert(p->n);
	return p->v + p->n;
}

inline size_t hash(const term& a) {
	return fnv(&a, sizeof a);
}

// The result of ordered comparison between different types is not really meaningful, but that's okay. It doesn't have to be
// meaningful, just consistent.
int cmp(term a, term b);

// Comparing symbols will not give alphabetical order, but an arbitrary order based on memory addresses, that is guaranteed to be
// consistent within a given process.
inline bool operator<(term a, term b) {
	return cmp(a, b) < 0;
}

inline bool operator<=(term a, term b) {
	return cmp(a, b) <= 0;
}

void print(tag t);
void print(term a);
