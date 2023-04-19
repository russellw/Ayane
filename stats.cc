#include "main.h"

#ifdef DEBUG
#ifdef _WIN32
#include <windows.h>
// Windows.h must be first.
#include <crtdbg.h>
#include <dbghelp.h>
#endif

namespace {
void printItem(size_t n, const char* caption) {
	auto s = buf + sizeof buf - 1;
	*s = 0;
	size_t i = 0;
	do {
		// Extract a digit.
		*--s = '0' + n % 10;
		n /= 10;

		// Track how many digits we have extracted.
		++i;

		// So that we can punctuate them in groups of 3.
		if (i % 3 == 0 && n) *--s = ',';
	} while (n);
	printf("%16s  %s\n", s, caption);
}

std::unordered_map<const char*, uint64_t> strStats;
std::unordered_map<size_t, uint64_t> numStats;
std::map<std::vector<const char*>, uint64_t> traces;
} // namespace

void incStat(const char* k, uint64_t n) {
	strStats[k] += n;
}

void incStat(size_t k, uint64_t n) {
	numStats[k] += n;
}

void incTrace() {
#ifdef _WIN32
	// Process.
	auto process = GetCurrentProcess();
	SymInitialize(process, 0, 1);

	// Stack frames.
	const int maxFrames = 64;
	static void* stack[maxFrames];
	auto nframes = CaptureStackBackTrace(1, maxFrames, stack, 0);

	// Symbol.
	auto si = (SYMBOL_INFO*)buf;
	si->MaxNameLen = 0x100;
	si->SizeOfStruct = sizeof(SYMBOL_INFO);

	// Location.
	IMAGEHLP_LINE64 loc;
	loc.SizeOfStruct = sizeof loc;

	// Trace.
	std::vector<const char*> v;
	for (int i = 0; i != nframes; ++i) {
		auto addr = (DWORD64)(stack[i]);
		SymFromAddr(process, addr, 0, si);
		DWORD displacement;
		*buf = 0;
		if (SymGetLineFromAddr64(process, addr, &displacement, &loc)) sprintf(buf, "%s:%d: ", loc.FileName, (int)loc.LineNumber);
		strcat(buf, si->Name);
		v.push_back(intern(buf)->v);
	}
	++traces[v];
#endif
}

void printStats() {
	putchar('\n');

	printItem(atoms->size(), "bytes atoms");
	printItem(compounds->size(), "bytes compounds");
	printItem(heap->size(), "bytes heap");
	putchar('\n');

	if (strStats.size()) {
		vec<const char*> v;
		for (auto p: strStats) v.push_back(p.first);
		sort(v.begin(), v.end(), [=](const char* a, const char* b) { return strcmp(a, b) < 0; });
		for (auto k: v) printItem(strStats[k], k);
		putchar('\n');
	}

	if (numStats.size()) {
		vec<size_t> v;
		for (auto p: numStats) v.push_back(p.first);
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
