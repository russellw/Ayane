// Arithmetic is polymorphic on integers and rationals
Expr* ceil(Expr* a);
Expr* floor(Expr* a);
Expr* trunc(Expr* a);
Expr* round(Expr* a);

// So is converting numbers between types
bool isInteger(Expr* a);

Expr* toInteger(Expr* a);
Expr* toRational(Expr* a);
Expr* toReal(Expr* a);
