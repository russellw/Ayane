#include "main.h"

// Numbers need to be interned, to preserve the property that equal terms have pointer equality, and in particular that they have
// the same hash codes

// TODO: divide this functionality between terms and simplify?
// Integers
namespace mpzs {
size_t cap = 4;
size_t qty;
atom** entries;

size_t hash(mpz_t a) {
	return mpz_get_ui(a);
}

bool eq(atom* a, mpz_t b) {
	return !mpz_cmp(a->mpz, b);
}

static size_t slot(atom** entries, size_t cap, mpz_t a) {
	size_t mask = cap - 1;
	auto i = hash(a) & mask;
	while (entries[i] && !eq(entries[i], a)) i = (i + 1) & mask;
	return i;
}

void expand() {
	assert(isPow2(cap));
	auto cap1 = cap * 2;
	auto entries1 = (atom**)xcalloc(cap1, sizeof *entries);
	// TODO: check generated code
	for (auto i = entries, e = entries + cap; i != e; ++i) {
		auto a = *i;
		if (a) entries1[slot(entries1, cap1, a->mpz)] = a;
	}
	free(entries);
	cap = cap1;
	entries = entries1;
}

struct init {
	init() {
		assert(isPow2(cap));
		entries = (atom**)xcalloc(cap, sizeof *entries);
	}
} _;
} // namespace mpzs

atom* intern(mpz_t a) {
	auto i = mpzs::slot(mpzs::entries, mpzs::cap, a);

	// If we have seen this before, return the existing object
	if (mpzs::entries[i]) {
		// TODO: cache result in local?
		mpz_clear(a);
		return mpzs::entries[i];
	}

	// Expand the hash table if necessary
	if (++mpzs::qty > mpzs::cap * 3 / 4) {
		mpzs::expand();
		i = mpzs::slot(mpzs::entries, mpzs::cap, a);
		assert(!mpzs::entries[i]);
	}

	// Make a new object
	auto r = (atom*)xmalloc(offsetof(atom, mpz) + sizeof(mpz_t));
	r->t = tag::Integer;
	memcpy(r->mpz, a, sizeof r->mpz);

	// Add to hash table
	return mpzs::entries[i] = r;
}

atom* integer(int n) {
	// TODO: name
	mpz_t r;
	mpz_init_set_si(r, n);
	return intern(r);
}

atom* integer(const char* s) {
	mpz_t r;
	if (mpz_init_set_str(r, s, 10)) err("Invalid integer");
	return intern(r);
}

// Rationals
namespace mpqs {
size_t cap = 4;
size_t qty;
atom** entries;

size_t hash(mpq_t a) {
	return hashCombine(mpz_get_ui(mpq_numref(a)), mpz_get_ui(mpq_denref(a)));
}

bool eq(atom* a, mpq_t b) {
	return mpq_equal(a->mpq, b);
}

static size_t slot(atom** entries, size_t cap, mpq_t a) {
	size_t mask = cap - 1;
	auto i = hash(a) & mask;
	while (entries[i] && !eq(entries[i], a)) i = (i + 1) & mask;
	return i;
}

void expand() {
	assert(isPow2(cap));
	auto cap1 = cap * 2;
	auto entries1 = (atom**)xcalloc(cap1, sizeof *entries);
	// TODO: check generated code
	for (auto i = entries, e = entries + cap; i != e; ++i) {
		auto a = *i;
		if (a) entries1[slot(entries1, cap1, a->mpq)] = a;
	}
	free(entries);
	cap = cap1;
	entries = entries1;
}

struct init {
	init() {
		assert(isPow2(cap));
		entries = (atom**)xcalloc(cap, sizeof *entries);
	}
} _;
} // namespace mpqs

atom* intern(mpq_t a) {
	auto i = mpqs::slot(mpqs::entries, mpqs::cap, a);

	// If we have seen this before, return the existing object
	if (mpqs::entries[i]) {
		// TODO: cache result in local?
		mpq_clear(a);
		return mpqs::entries[i];
	}

	// Expand the hash table if necessary
	if (++mpqs::qty > mpqs::cap * 3 / 4) {
		mpqs::expand();
		i = mpqs::slot(mpqs::entries, mpqs::cap, a);
		assert(!mpqs::entries[i]);
	}

	// Make a new object
	auto r = (atom*)xmalloc(offsetof(atom, mpq) + sizeof(mpq_t));
	r->t = tag::Rational;
	memcpy(r->mpq, a, sizeof r->mpq);

	// Add to hash table
	return mpqs::entries[i] = r;
}

atom* rational(int n, unsigned d) {
	mpq_t r;
	mpq_init(r);
	mpq_set_si(r, n, d);
	mpq_canonicalize(r);
	return intern(r);
}

atom* rational(const char* s) {
	mpq_t r;
	mpq_init(r);
	if (mpq_set_str(r, s, 10)) err("Invalid rational");
	mpq_canonicalize(r);
	return intern(r);
}

atom* real(mpq_t q) {
	return mk(tag::ToReal, intern(q));
}

atom* real(int n, unsigned d) {
	mpq_t r;
	mpq_init(r);
	mpq_set_si(r, n, d);
	mpq_canonicalize(r);
	return real(r);
}

atom* real(const char* s) {
	// GMP string to integer or rational doesn't handle leading +, so for consistency, this function doesn't either
	assert(*s != '+');

	// Sign
	bool sign = 0;
	if (*s == '-') {
		++s;
		sign = 1;
	}

	// Result = scaled mantissa
	mpq_t r;
	mpq_init(r);
	auto mantissa = mpq_numref(r);
	auto powScale = mpq_denref(r);

	// Integer part
	mpz_t integerPart;
	mpz_init(integerPart);
	auto t = s;
	if (isDigit(*t)) {
		do ++t;
		while (isDigit(*t));
		bufCopy(s, t);
		if (mpz_set_str(integerPart, buf, 10)) err("Invalid integer part");
		s = t;
	}

	// Decimal part
	size_t scale = 0;
	if (*s == '.') {
		++s;
		t = s;
		if (isDigit(*t)) {
			do ++t;
			while (isDigit(*t));
			bufCopy(s, t);
			if (mpz_set_str(mantissa, buf, 10)) err("Invalid decimal part");
			scale = t - s;
			s = t;
		}
	}
	mpz_ui_pow_ui(powScale, 10, scale);

	// Mantissa += integerPart * 10^scale
	mpz_addmul(mantissa, integerPart, powScale);

	// Sign
	if (sign) mpz_neg(mantissa, mantissa);

	// Exponent
	bool exponentSign = 0;
	auto exponent = 0UL;
	if (*s == 'e' || *s == 'E') {
		++s;
		switch (*s) {
		case '-':
			exponentSign = 1;
			[[fallthrough]];
		case '+':
			++s;
			break;
		}
		errno = 0;
		exponent = strtoul(s, 0, 10);
		if (errno) err(strerror(errno));
	}
	mpz_t powExponent;
	mpz_init(powExponent);
	mpz_ui_pow_ui(powExponent, 10, exponent);
	if (exponentSign) mpz_mul(powScale, powScale, powExponent);
	else
		mpz_mul(mantissa, mantissa, powExponent);

	// Reduce result to lowest terms
	mpq_canonicalize(r);

	// Cleanup
	// TODO: free in reverse order?
	mpz_clear(powExponent);
	mpz_clear(integerPart);

	// Wrap result in term designating it as a real number
	return real(r);
}

// Arithmetic
namespace {
void mpz_ediv_r(mpz_t r, const mpz_t n, const mpz_t d) {
	mpz_tdiv_r(r, n, d);
	if (mpz_sgn(r) < 0) {
		mpz_t dabs;
		mpz_init(dabs);
		mpz_abs(dabs, d);
		mpz_add(r, r, dabs);
		mpz_clear(dabs);
	}
}

void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d) {
	mpz_t r;
	mpz_init(r);
	mpz_ediv_r(r, n, d);
	mpz_sub(q, n, r);
	mpz_clear(r);
	mpz_tdiv_q(q, q, d);
}

// Calculate q = n/d, assuming common factors have already been canceled out, and applying bankers rounding
void mpz_round(mpz_t q, mpz_t n, mpz_t d) {
	// If we are dividing by 2, the result could be exactly halfway between two integers, so need special case to apply bankers
	// rounding
	if (!mpz_cmp_ui(d, 2)) {
		// Floored division by 2 (this corresponds to arithmetic shift right one bit)
		mpz_fdiv_q_2exp(q, n, 1);

		// If it was an even number before the division, the issue doesn't arise; we already have the exact answer
		if (!mpz_tstbit(n, 0)) return;

		// If it's an even number after the division, we are already on the nearest even integer, so we don't need to do anything
		// else
		if (!mpz_tstbit(q, 0)) return;

		// Need to adjust by one to land on an even integer, but which way? Floored division rounded down, so we need to go up.
		mpz_add_ui(q, q, 1);
		return;
	}

	// We are not dividing by 2, so cannot end up exactly halfway between two integers, and merely need to add half the denominator
	// to the numerator before dividing
	mpz_t d2;
	mpz_init(d2);
	mpz_fdiv_q_2exp(d2, d, 1);
	mpz_add(q, n, d2);
	mpz_clear(d2);
	mpz_fdiv_q(q, q, d);
}
} // namespace

atom* neg(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_neg(r, a1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpq_t r;
		mpq_init(r);
		mpq_neg(r, a1);
		return intern(r);
	}
	}
	unreachable;
}

atom* add(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_add(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();
		mpq_t r;
		mpq_init(r);
		mpq_add(r, a1, b1);
		return intern(r);
	}
	}
	unreachable;
}

atom* sub(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_sub(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();
		mpq_t r;
		mpq_init(r);
		mpq_sub(r, a1, b1);
		return intern(r);
	}
	}
	unreachable;
}

atom* mul(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_mul(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();
		mpq_t r;
		mpq_init(r);
		mpq_mul(r, a1, b1);
		return intern(r);
	}
	}
	unreachable;
}

atom* div(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);

		// TPTP does not define integer division with unspecified rounding mode, but most programming languages nowadays define it
		// as truncating. Todo: Does SMT-LIB use this?
		mpz_tdiv_q(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();
		mpq_t r;
		mpq_init(r);
		mpq_div(r, a1, b1);
		return intern(r);
	}
	}
	unreachable;
}

atom* divT(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_tdiv_q(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();

		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a1), mpq_denref(b1));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a1), mpq_numref(b1));

		mpq_t r;
		mpq_init(r);
		mpz_tdiv_q(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

atom* divF(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_fdiv_q(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();

		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a1), mpq_denref(b1));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a1), mpq_numref(b1));

		mpq_t r;
		mpq_init(r);
		mpz_fdiv_q(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

atom* divE(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_ediv_q(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();

		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a1), mpq_denref(b1));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a1), mpq_numref(b1));

		mpq_t r;
		mpq_init(r);
		mpz_ediv_q(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

atom* remT(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_tdiv_r(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();

		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a1), mpq_denref(b1));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a1), mpq_numref(b1));

		mpq_t r;
		mpq_init(r);
		mpz_tdiv_r(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

atom* remF(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_fdiv_r(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();

		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a1), mpq_denref(b1));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a1), mpq_numref(b1));

		mpq_t r;
		mpq_init(r);
		mpz_fdiv_r(mpq_numref(r), xnum_yden, xden_ynum);

		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

atom* remE(atom* a, atom* b) {
	assert(tag(a) == tag(b));
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		auto b1 = b.mpz();
		mpz_t r;
		mpz_init(r);
		mpz_ediv_r(r, a1, b1);
		return intern(r);
	}
	case tag::Rational:
	{
		auto a1 = a.mpq();
		auto b1 = b.mpq();

		mpz_t xnum_yden;
		mpz_init(xnum_yden);
		mpz_mul(xnum_yden, mpq_numref(a1), mpq_denref(b1));

		mpz_t xden_ynum;
		mpz_init(xden_ynum);
		mpz_mul(xden_ynum, mpq_denref(a1), mpq_numref(b1));

		mpq_t r;
		mpq_init(r);
		mpz_ediv_r(mpq_numref(r), xnum_yden, xden_ynum);

		// TODO: free in reverse order?
		mpz_clear(xden_ynum);
		mpz_clear(xnum_yden);
		return intern(r);
	}
	}
	unreachable;
}

atom* ceil(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
		return a;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpq_t r;
		mpq_init(r);
		mpz_cdiv_q(mpq_numref(r), mpq_numref(a1), mpq_denref(a1));
		return intern(r);
	}
	}
	unreachable;
}

atom* floor(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
		return a;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpq_t r;
		mpq_init(r);
		mpz_fdiv_q(mpq_numref(r), mpq_numref(a1), mpq_denref(a1));
		return intern(r);
	}
	}
	unreachable;
}

atom* trunc(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
		return a;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpq_t r;
		mpq_init(r);
		mpz_tdiv_q(mpq_numref(r), mpq_numref(a1), mpq_denref(a1));
		return intern(r);
	}
	}
	unreachable;
}

atom* round(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
		return a;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpq_t r;
		mpq_init(r);
		mpz_round(mpq_numref(r), mpq_numref(a1), mpq_denref(a1));
		return intern(r);
	}
	}
	unreachable;
}

bool isInteger(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
		return 1;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		return !mpz_cmp_ui(mpq_denref(a1), 1);
	}
	}
	unreachable;
}

atom* toInteger(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
		return a;
	case tag::Rational:
	{
		auto a1 = a.mpq();
		mpz_t r;
		mpz_init(r);

		// Different languages have different conventions on the default rounding mode for converting fractions to integers. TPTP
		// defines it as floor, so that is used here. To use a different rounding mode, explicity round the rational number first,
		// and then convert to integer.
		mpz_fdiv_q(r, mpq_numref(a1), mpq_denref(a1));
		return intern(r);
	}
	}
	unreachable;
}

atom* toRational(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		mpq_t r;
		mpq_init(r);
		mpz_set(mpq_numref(r), a1);
		return intern(r);
	}
	case tag::Rational:
		return a;
	}
	unreachable;
}

atom* toReal(atom* a) {
	switch (tag(a)) {
	case tag::Integer:
	{
		auto a1 = a.mpz();
		mpq_t r;
		mpq_init(r);
		mpz_set(mpq_numref(r), a1);
		return real(r);
	}
	case tag::Rational:
		return intern(tag::ToReal, a);
	}
	unreachable;
}
