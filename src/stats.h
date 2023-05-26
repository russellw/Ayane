#ifdef DBG
void incStat(const char* s, uint64_t n = 1);
void incStat(size_t s, uint64_t n = 1);
void incTrace();
void printStats();
#else
inline void incStat(const char* s, uint64_t n = 1) {
}
inline void incStat(size_t s, uint64_t n = 1) {
}
inline void incTrace() {
}
inline void printStats() {
}
#endif
