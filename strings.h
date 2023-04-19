// Strings are interned for fast comparison, and fast access to associated types and symbols. The latter are stored as raw offsets
// rather than in their typed wrappers, to make it possible to statically initialize the array of keywords.
struct string {
	uint32_t dobj;
	uint32_t sym;
	uint32_t ty;

	// Although the allocated size of dynamically allocated strings will vary according to the number of characters needed, the
	// declared size of the character array needs to be positive for the statically allocated array of known strings (keywords). It
	// needs to be large enough to accommodate the longest keyword plus null terminator. And the size of the whole structure should
	// be a power of 2 because keyword() needs to divide by that size.
	char v[32 - 4 - 4 - 4];
};

// Keywords are strings that are known to be important.
enum
{
	s_question,
	s_C,
	s_T,
	s_V,
	s_ax,
	s_bool,
	s_break,
	s_cc,
	s_ceiling,
	s_clause,
	s_cnf,
	s_conjecture,
	s_continue,
	s_cpp,
	s_cpulimit,
	s_cxx,
	s_difference,
	s_dimacs,
	s_dimacsin,
	s_dimacsout,
	s_distinct,
	s_do,
	s_else,
	s_false,
	s_floor,
	s_fof,
	s_for,
	s_graph,
	s_greater,
	s_greatereq,
	s_h,
	s_help,
	s_i,
	s_if,
	s_in,
	s_include,
	s_int,
	s_is_int,
	s_is_rat,
	s_less,
	s_lesseq,
	s_m,
	s_map,
	s_memory,
	s_memorylimit,
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
	s_return,
	s_round,
	s_set,
	s_sum,
	s_t,
	s_tType,
	s_tff,
	s_to_int,
	s_to_rat,
	s_to_real,
	s_tptp,
	s_tptpin,
	s_tptpout,
	s_true,
	s_truncate,
	s_type,
	s_uminus,
	s_val,
	s_vector,
	s_version,
	s_void,
	s_while,
	end_s
};

// And statically allocated for fast lookup.
extern string keywords[];

inline size_t keyword(const string* s) {
	// Assign the difference to an unsigned variable and perform the division explicitly, because ptrdiff_t is a signed type, but
	// unsigned division is slightly faster.
	size_t i = (char*)s - (char*)keywords;
	return i / sizeof(string);
}

// Clear term fields of every existing string. If reading multiple problems, call this before each. Type fields are not cleared,
// because the set of known types itself is not cleared between problems, because types are typically few enough to make this
// unnecessary.
void clearStrings();

string* intern(const char* s, size_t n);
inline string* intern(const char* s) {
	return intern(s, strlen(s));
}

inline void print(const string* s) {
	print(s->v);
}
