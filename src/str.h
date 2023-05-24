// Strings are interned for fast comparison, and fast access to associated types and values
struct Str {
	Fn* fn;
	OpaqueType* ty;

	// Although the allocated size of dynamically allocated strings will vary according to the number of characters needed, the
	// declared size of the character array needs to be positive for the statically allocated array of known strings (keywords). It
	// needs to be large enough to accommodate the longest keyword plus null terminator. And the size of the whole structure should
	// be a power of 2 because keyword lookup needs to divide by that size.
	char v[32 - 2 * sizeof(void*)];
};

// Keywords are strings that are known to be important
enum {
	s_declare_fun,
	s_assert,
	s_ax,
	s_smt2,
	s_ite,
	s_Int,
	s_bool,
	s_not,
	s_and,
	s_or,
	s_xor,
	s_thf,
	s_ceiling,
	s_set_info,
	s_set_logic,
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
	s_tcf,
	s_uminus,
	s_check_sat,
	s_exit,
	nkeywords
};

// And statically allocated for fast lookup
extern Str keywords[];

Str* intern(char* s, size_t n);
inline Str* intern(const char* s) {
	return intern((char*)s, strlen(s));
}

inline void print(Str* s) {
	print(s->v);
}
