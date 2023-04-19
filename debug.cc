#include "main.h"

#ifdef DEBUG
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

	// Print.
	for (int i = 0; i != nframes; ++i) {
		auto addr = (DWORD64)(stack[i]);
		SymFromAddr(process, addr, 0, si);
		DWORD displacement;
		if (SymGetLineFromAddr64(process, addr, &displacement, &loc)) fprintf(stderr, "%s:%d: ", loc.FileName, (int)loc.LineNumber);
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
