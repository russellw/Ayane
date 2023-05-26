// In typed first-order logic, we can name a type, and then work with it without needing to know what it is made of
struct OpaqueType: LeafType {
	char* s;

	OpaqueType(char* s): LeafType(Kind::opaque), s(s) {
	}
};
