void initBignums();

// Functions for making arbitrary precision numbers for convenience, accept integer or string input and will intern the result so
// equality tests can simply compare pointers.
term integer(int n);
term integer(const char* s);

term rational(int n, unsigned d);
term rational(const char* s);

// Real number literals are represented as rational number literals wrapped in ToReal. It's a function call that is not actually
// evaluated, since there is no representation of real number literals as such.
term real(mpq_t q);
term real(int n, unsigned d);

// Per TPTP syntax, decimal/exponent string parses to a real number literal.
term real(const char* s);

// Arithmetic is polymorphic on integers and rationals.
term operator-(term a);
term operator+(term a, term b);
term operator-(term a, term b);
term operator*(term a, term b);
term operator/(term a, term b);

term divT(term a, term b);
term divF(term a, term b);
term divE(term a, term b);
term remT(term a, term b);
term remF(term a, term b);
term remE(term a, term b);

term ceil(term a);
term floor(term a);
term trunc(term a);
term round(term a);

// So is converting numbers between types.
bool isInteger(term a);

term toInteger(term a);
term toRational(term a);
term toReal(term a);
