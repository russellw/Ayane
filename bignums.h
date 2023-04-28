atom* intern(mpz_t a);
atom* intern(mpq_t a);

// Functions for making arbitrary precision numbers for convenience, accept integer or string input and will intern the result so
// equality tests can simply compare pointers
atom* integer(int n);
atom* integer(const char* s);

atom* rational(int n, unsigned d);
atom* rational(const char* s);

// Real number literals are represented as rational number literals wrapped in ToReal. It's a function call that is not actually
// evaluated, since there is no representation of real number literals as such.
Term* real(mpq_t q);
Term* real(int n, unsigned d);

// Per TPTP syntax, decimal/exponent string parses to a real number literal
Term* real(const char* s);

// Arithmetic is polymorphic on integers and rationals
atom* neg(atom* a);
atom* add(atom* a, atom* b);
atom* sub(atom* a, atom* b);
atom* mul(atom* a, atom* b);
atom* div(atom* a, atom* b);

atom* divT(atom* a, atom* b);
atom* divF(atom* a, atom* b);
atom* divE(atom* a, atom* b);
atom* remT(atom* a, atom* b);
atom* remF(atom* a, atom* b);
atom* remE(atom* a, atom* b);

atom* ceil(atom* a);
atom* floor(atom* a);
atom* trunc(atom* a);
atom* round(atom* a);

// So is converting numbers between types
bool isInteger(atom* a);

atom* toInteger(atom* a);
atom* toRational(atom* a);
Term* toReal(atom* a);
