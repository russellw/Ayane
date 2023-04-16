enum class kind
{
#define _(x) x,
#include "kinds.h"
};

// The first few numbers are reserved for unboxed atomic types, for which the type number is just the kind, cast to an integer.
const size_t unboxedTypes = (size_t)kind::Void + 1;

// The total span of type numbers is 2^typeBits. The number of bits is important for terms, which sometimes want to pack a type with
// other information into a word.
const int typeBits = 17;

struct Type {
	kind k;
	uint32_t n;
	union {
		const char* s;
		// TODO: does the vector need to contain raw offsets?
		uint32_t v[0];
	};
};

// The number of types used in any run is expected to be small enough, that a statically allocated array will suffice for all of
// them.
extern char typeMem[];

// It is desirable to pack types into as few bits as possible, for efficient term representation. Since types are always allocated
// in groups of 64-bit words, they can be represented as word offsets, which saves three bits.
inline Type* typePtr(size_t o) {
	assert(o < 1 << typeBits);
	return (Type*)(typeMem + o * 8);
}

struct type {
	uint32_t offset;

	// Unboxed types.
	type(kind k = kind::Unknown) {
		assert((size_t)k < unboxedTypes);
		offset = (size_t)k * offsetof(Type, v) / 8;
	}

	// Boxed atomic types.
	explicit type(string* s);

	// Compound types are interned.
	explicit type(kind k, type a);
	explicit type(kind k, type a, type b);
	explicit type(kind k, const vec<type>& v);

	// Terms need the ability to reconstitute types from bits.
	explicit type(size_t offset): offset(offset) {
	}

	// Even unboxed types are allocated space in the (first few words of the) array, which means the kind of any type can be
	// recovered in the same way, by reading the first word of the corresponding Type object.
	// TODO: optimize
	explicit operator kind() const {
		return typePtr(offset)->k;
	}

	// Compound operations.
	size_t size() const {
		return typePtr(offset)->n;
	}

	type operator[](size_t i) const {
		auto p = typePtr(offset);
		assert(i < p->n);
		return type(p->v[i]);
	}
};

inline size_t hash(type ty) {
	return ty.offset;
}

// Because compound types are interned, equality can simply compare words.
inline bool operator==(type a, type b) {
	return a.offset == b.offset;
}

inline bool operator!=(type a, type b) {
	return !(a == b);
}

// Try to assign one type from another, doing the right thing if either is unknown. If both are known and different, raise an error.
void assign(type& t, type u);

void print(kind k);
void print(type ty);
