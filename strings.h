// Strings are interned for fast comparison, and fast access to associated types and symbols. The latter are stored as raw offsets
// rather than in their typed wrappers, to make it possible to statically initialize the array of keywords.
struct string {
	// TODO: move some of these to map objects?
	uint32_t dobj;
	uint32_t sym;
	uint32_t ty;

	// Although the allocated size of dynamically allocated strings will vary according to the number of characters needed, the
	// declared size of the character array needs to be positive for the statically allocated array of known strings (keywords). It
	// needs to be large enough to accommodate the longest keyword plus null terminator. And the size of the whole structure should
	// be a power of 2 because keyword() needs to divide by that size.
	char v[32 - 4 - 4 - 4];
};

// Keywords are strings that are known to be important
enum {
	s_ax,
	s_bool,
	s_ceiling,
	s_cnf,
	s_conjecture,
	s_difference,
	s_distinct,
	s_false,
	s_floor,
	s_fof,
	s_greater,
	s_greatereq,
	s_i,
	s_include,
	s_int,
	s_is_int,
	s_is_rat,
	s_less,
	s_lesseq,
	s_o,
	s_p,
	s_product,
	s_quotient,
	s_quotient_e,
	s_quotient_f,
	s_quotient_t,
	s_rat,
	s_real,
	s_remainder_e,
	s_remainder_f,
	s_remainder_t,
	s_round,
	s_sum,
	s_tType,
	s_tff,
	s_to_int,
	s_to_rat,
	s_to_real,
	s_true,
	s_truncate,
	s_type,
	s_uminus,
	end_s
};

// And statically allocated for fast lookup
extern string keywords[];

inline size_t keyword(const string* s) {
	// Assign the difference to an unsigned variable and perform the division explicitly, because ptrdiff_t is a signed type, but
	// unsigned division is slightly faster
	size_t i = (char*)s - (char*)keywords;
	return i / sizeof(string);
}

string* intern(const char* s, size_t n);
inline string* intern(const char* s) {
	return intern(s, strlen(s));
}

inline void print(const string* s) {
	print(s->v);
}
