#ifdef DEBUG
void incStat(const char* k, uint64_t n = 1);
void incStat(size_t k, uint64_t n = 1);
void incTrace();
void printStats();
#else
inline void incStat(const char* k, uint64_t n = 1) {
}

inline void incStat(size_t k, uint64_t n = 1) {
}

#define incTrace()
#define printStats()
#endif
