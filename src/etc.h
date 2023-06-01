// Error codes specified by this program have numbers starting at 200 to avoid overlap with those specified by POSIX or Windows.
// Numbers are assigned roughly in order of increasing severity

// This can happen even when the input is correct, e.g. problem contains higher-order logic that the system does not understand,
// 'inappropriate' in SZS terminology
const int inappropriateError = 200;

// This means an error in the input
const int inputError = 201;

// This means a bug in the code
const int assertError = 202;

// SORT
inline size_t hashCombine(size_t a, size_t b) { return a ^ b + 0x9e3779b9u + (a << 6) + (a >> 2); }
inline size_t roundUp(size_t n, size_t alignment) { return (n + alignment - 1) & ~(alignment - 1); }
//

// SORT
size_t fnv(const char* s);
size_t fnv(const void* p, size_t n);
void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d);
void mpz_ediv_r(mpz_t r, const mpz_t n, const mpz_t d);
void mpz_round(mpz_t q, const mpz_t n, const mpz_t d);
void* ialloc(size_t n);
