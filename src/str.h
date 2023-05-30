// Strings are interned for fast comparison, and fast access to associated types and values
struct Str {
	Type* t;
	Fn* fn;

	// Although the allocated size of dynamically allocated strings will vary according to the number of characters needed, the
	// declared size of the character array needs to be positive for the statically allocated array of known strings (keywords). It
	// needs to be large enough to accommodate the longest keyword plus null terminator. And the size of the whole structure should
	// be a power of 2 because keyword lookup needs to divide by that size.
	char v[64 - 2 * sizeof(void*)];
};

// Keywords are strings that are known to be important
enum {
	s_Bool,
	s_Float32,
	s_Float64,
	s_Int,
	s_Real,
	s_RoundingMode,
	s_String,
	s_and,
	s_assert,
	s_ax,
	s_bool,
	s_bvand,
	s_bvnot,
	s_bvor,
	s_bvsdiv,
	s_bvsmod,
	s_bvxor,
	s_ceiling,
	s_check_sat,
	s_cnf,
	s_concat,
	s_conjecture,
	s_declare_const,
	s_declare_datatype,
	s_declare_datatypes,
	s_declare_fun,
	s_declare_sort,
	s_define_fun,
	s_define_sort,
	s_difference,
	s_distinct,
	s_div,
	s_exists,
	s_false,
	s_floor,
	s_fof,
	s_forall,
	s_fp_eq,
	s_fp_lt,
	s_greater,
	s_greatereq,
	s_i,
	s_include,
	s_int,
	s_is_int,
	s_is_rat,
	s_ite,
	s_less,
	s_lesseq,
	s_let,
	s_mod,
	s_not,
	s_o,
	s_or,
	s_p,
	s_product,
	s_push,
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
	s_set_info,
	s_set_logic,
	s_smt2,
	s_sum,
	s_tType,
	s_tcf,
	s_tff,
	s_thf,
	s_to_int,
	s_to_rat,
	s_to_real,
	s_true,
	s_truncate,
	s_type,
	s_uminus,
	s_xor,
	s_bang,
	s_star,
	s_plus,
	s_minus,
	s_slash,
	s_lt,
	s_le,
	s_eq,
	s_imp,
	s_gt,
	s_ge,
	s_underscore,
};

// And statically allocated for fast lookup
extern Str keywords[];

Str* intern(char* s, size_t n);
inline Str* intern(const char* s) {
	return intern((char*)s, strlen(s));
}

#ifdef DBG
inline void dbgPrint(Str* s) {
	dbgPrint(s->v);
}
#endif
