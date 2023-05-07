#include "main.h"

#ifdef DBG
#ifdef _WIN32
#include <windows.h>
// Windows.h must be first.
#include <crtdbg.h>
#include <dbghelp.h>
#endif

#include <map>

namespace {
void printItem(size_t n, const char* caption) {
	char buf[32];
	auto s = buf + sizeof buf - 1;
	*s = 0;
	size_t i = 0;
	do {
		// Extract a digit
		*--s = '0' + n % 10;
		n /= 10;

		// Track how many digits we have extracted
		++i;

		// So that we can punctuate them in groups of 3
		if (i % 3 == 0 && n) *--s = ',';
	} while (n);
	printf("%16s  %s\n", s, caption);
}

unordered_map<const char*, uint64_t> strStats;
unordered_map<size_t, uint64_t> numStats;
// TODO: does this map need to be ordered?
std::map<vector<const char*>, uint64_t> traces;
} // namespace

void incStat(const char* k, uint64_t n) {
	strStats[k] += n;
}

void incStat(size_t k, uint64_t n) {
	numStats[k] += n;
}

void incTrace() {
#ifdef _WIN32
	// Process
	auto process = GetCurrentProcess();
	SymInitialize(process, 0, 1);

	// Stack frames
	const int maxFrames = 100;
	static void* stack[maxFrames];
	auto nframes = CaptureStackBackTrace(1, maxFrames, stack, 0);

	// Symbol
	char si0[sizeof(SYMBOL_INFO) + MAX_SYM_NAME - 1];
	auto si = (SYMBOL_INFO*)si0;
	si->SizeOfStruct = sizeof(SYMBOL_INFO);
	si->MaxNameLen = MAX_SYM_NAME;

	// Location
	IMAGEHLP_LINE64 loc;
	loc.SizeOfStruct = sizeof loc;

	// Trace
	vector<const char*> v;
	for (int i = 0; i < nframes; ++i) {
		auto addr = (DWORD64)(stack[i]);
		SymFromAddr(process, addr, 0, si);
		DWORD displacement;
		char* s;
		if (SymGetLineFromAddr64(process, addr, &displacement, &loc)) {
			snprintf(buf, sizeof buf, "%s:%u: %s", loc.FileName, loc.LineNumber, si->Name);
			s = buf;
		} else
			s = si->Name;
		v.push_back(intern(s)->v);
	}
	++traces[v];
#endif
}

void printStats() {
	putchar('\n');

	if (strStats.size()) {
		vec<const char*> v;
		for (auto p: strStats) v.add(p.first);
		sort(v.begin(), v.end(), [=](const char* a, const char* b) { return strcmp(a, b) < 0; });
		for (auto k: v) printItem(strStats[k], k);
		putchar('\n');
	}

	if (numStats.size()) {
		vec<size_t> v;
		for (auto p: numStats) v.add(p.first);
		sort(v.begin(), v.end());
		uint64_t totQty = 0;
		uint64_t totVal = 0;
		for (auto val: v) {
			sprintf(buf, "%zu", val);
			auto qty = numStats[val];
			printItem(qty, buf);
			totQty += qty;
			totVal += val * qty;
		}
		printItem(totQty, "qty");
		printf("%16.3f  avg", totVal / double(totQty));
		putchar('\n');
	}

	for (auto& kv: traces) {
		print(kv.second);
		putchar('\n');
		for (auto s: kv.first) printf("\t%s\n", s);
		putchar('\n');
	}
}
#endif
