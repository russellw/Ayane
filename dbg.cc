#include "main.h"

#ifdef DBG
#ifdef _WIN32
#include <windows.h>
// Windows.h must be first.
#include <crtdbg.h>
#include <dbghelp.h>
#endif

static int level;

Tracer::Tracer() {
	++level;
}

Tracer::~Tracer() {
	--level;
}

void indent() {
	for (auto i = level; i--;) print("  ");
}

void stackTrace() {
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

	// Print
	for (int i = 0; i < nframes; ++i) {
		auto addr = (DWORD64)(stack[i]);
		SymFromAddr(process, addr, 0, si);
		DWORD displacement;
		if (SymGetLineFromAddr64(process, addr, &displacement, &loc)) fprintf(stderr, "%s:%u: ", loc.FileName, loc.LineNumber);
		fprintf(stderr, "%s\n", si->Name);
	}
#endif
}

bool assertFail(const char* file, int line, const char* func, const char* s) {
	fprintf(stderr, "%s:%d: %s: Assert failed: %s\n", file, line, func, s);
	stackTrace();
	exit(1);
}
#endif
