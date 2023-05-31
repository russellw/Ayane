#include "main.h"

#ifdef DBG

#ifdef _WIN32
#include <windows.h>

#include <crtdbg.h>
#include <dbghelp.h>
#else
extern "C" const char* __asan_default_options() { return "detect_leaks=0"; }
#endif

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
	auto si = (SYMBOL_INFO*)buf;
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
		if (SymGetLineFromAddr64(process, addr, &displacement, &loc)) printf("%s:%lu: ", loc.FileName, loc.LineNumber);
		printf("%s\n", si->Name);
	}
#endif
}

bool assertFail(const char* file, int line, const char* func, const char* s) {
	printf("%s:%d: %s: assert failed: %s\n", file, line, func, s);
	stackTrace();
	exit(assertError);
}

#endif
