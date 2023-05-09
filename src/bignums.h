// Arithmetic is polymorphic on integers and rationals
Expr* minus(Expr* a);
Expr* div(Expr* a, Expr* b);

Expr* divTrunc(Expr* a, Expr* b);
Expr* divFloor(Expr* a, Expr* b);
Expr* divEuclid(Expr* a, Expr* b);
Expr* remTrunc(Expr* a, Expr* b);
Expr* remFloor(Expr* a, Expr* b);
Expr* remEuclid(Expr* a, Expr* b);

Expr* ceil(Expr* a);
Expr* floor(Expr* a);
Expr* trunc(Expr* a);
Expr* round(Expr* a);

// So is converting numbers between types
bool isInteger(Expr* a);

Expr* toInteger(Expr* a);
Expr* toRational(Expr* a);
Expr* toReal(Expr* a);
