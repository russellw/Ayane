#include "main.h"

#ifdef DBG
#ifdef _WIN32
#include <windows.h>

#include <crtdbg.h>
#include <dbghelp.h>
#endif

#include <map>

namespace {
unordered_map<const char*, uint64_t> strStats;
unordered_map<size_t, uint64_t> numStats;
// TODO: does this map need to be ordered?
std::map<vector<const char*>, uint64_t> traces;
} // namespace

void incStat(const char* s, uint64_t n) {
	strStats[intern(s)->v] += n;
}

void incStat(size_t s, uint64_t n) {
	numStats[s] += n;
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
			snprintf(buf, bufSize, "%s:%lu: %s", loc.FileName, loc.LineNumber, si->Name);
			s = buf;
		} else
			s = si->Name;
		v.push_back(intern(s)->v);
	}
	++traces[v];
#endif
}

void printStats() {
	if (strStats.size()) {
		Vec<const char*> v;
		for (auto ab: strStats) v.add(ab.first);
		sort(v.begin(), v.end(), [=](const char* a, const char* b) { return strcmp(a, b) < 0; });
		for (auto s: v) printf(";%s\t%zu\n", s, strStats[s]);
	}
	if (numStats.size()) {
		Vec<size_t> v;
		for (auto ab: numStats) v.add(ab.first);
		sort(v.begin(), v.end());
		size_t tot = 0;
		size_t totn = 0;
		for (auto a: v) {
			auto n = numStats[a];
			printf(";%zu\t%zu\n", a, n);
			tot += a * n;
			totn += n;
		}
		printf(";tot\t%zu\n", totn);
		printf(";avg\t%.3f", tot / (double)totn);
	}
	for (auto& ab: traces) {
		putchar('\n');
		print(ab.second);
		putchar('\n');
		for (auto s: ab.first) printf("\t%s\n", s);
	}
}
#endif
