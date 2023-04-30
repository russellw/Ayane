#include "main.h"

#ifdef _WIN32
#include <windows.h>

namespace {
LONG WINAPI handler(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		WriteFile(GetStdHandle(STD_ERROR_HANDLE), "Stack overflow\n", 15, 0, 0);
	else {
		fprintf(stderr, "Exception code %lx\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
		stackTrace();
	}
	ExitProcess(ExceptionInfo->ExceptionRecord->ExceptionCode);
}

VOID CALLBACK timeout(PVOID a, BOOLEAN b) {
	ExitProcess(7);
}
} // namespace
#else
#include <unistd.h>
#endif

#define version "3"

enum {
	dimacs = 1,
	tptp,
};

namespace {
const char* ext(const char* file) {
	auto s = strrchr(file, '.');
	return s ? s + 1 : "";
}

const char* optArg(int argc, const char** argv, int& i, const char* oa) {
	if (*oa) return oa;
	if (i + 1 == argc) {
		fprintf(stderr, "%s: Expected arg\n", argv[i]);
		exit(1);
	}
	return argv[++i];
}

double optDouble(int argc, const char** argv, int& i, const char* oa) {
	oa = optArg(argc, argv, i, oa);
	errno = 0;
	auto r = strtod(oa, 0);
	if (errno) {
		perror(oa);
		exit(1);
	}
	return r;
}

// SORT
const char* file;
int inputLanguage;
int outputLanguage;
///

void parse(int argc, const char** argv) {
	for (int i = 0; i != argc; ++i) {
		auto s = argv[i];

		// File
		if (*s != '-') {
			files.add(s);
			continue;
		}

		// Option
		bufi = 0;
		auto oa = "";
		for (;;) {
			if (isAlpha(*s) && isDigit(s[1])) {
				bufAdd(*s);
				oa = s + 1;
				break;
			}
			switch (*s) {
			case ':':
			case '=':
				oa = s + 1;
				break;
			case 0:
				break;
			default:
				bufAdd(*s);
				[[fallthrough]];
			case '-':
				++s;
				continue;
			}
			break;
		}

		// An unadorned '-' means read from standard input, but that's the default anyway if no files are specified, so quietly
		// accept it
		if (!bufi) continue;

		// Option
		switch (keyword(intern(buf, bufi))) {
		case s_cpulimit:
		case s_T:
		case s_t:
		{
			auto seconds = optDouble(argc, argv, i, oa);
#ifdef _WIN32
			HANDLE timer = 0;
			CreateTimerQueueTimer(&timer, 0, timeout, 0, (DWORD)(seconds * 1000), 0, WT_EXECUTEINTIMERTHREAD);
#else
			alarm(seconds);
#endif
			continue;
		}
		case s_dimacs:
			inputLanguage = dimacs;
			outputLanguage = dimacs;
			continue;
		case s_dimacsin:
			inputLanguage = dimacs;
			continue;
		case s_dimacsout:
			outputLanguage = dimacs;
			continue;
		case s_h:
		case s_help:
			printf(
				// SORT
				"-dimacs      Set DIMACS as input and output format\n"
				"-dimacs-in   Set DIMACS as input format\n"
				"-dimacs-out  Set DIMACS as output format\n"
				"-h           Show help\n"
				"-t seconds   Time limit\n"
				"-tptp        Set TPTP as input and output format\n"
				"-tptp-in     Set TPTP as input format\n"
				"-tptp-out    Set TPTP as output format\n"
				"-V           Show version\n"
				///
			);
			exit(0);
		case s_tptp:
			inputLanguage = tptp;
			outputLanguage = tptp;
			continue;
		case s_tptpin:
			inputLanguage = tptp;
			continue;
		case s_tptpout:
			outputLanguage = tptp;
			continue;
		case s_V:
		case s_version:
			printf("Ayane " version);
			if (sizeof(void*) == 4) printf(", 32 bits");
#ifdef DEBUG
			printf(", debug build");
#endif
			putchar('\n');
			exit(0);
		}
		fprintf(stderr, "%s: Unknown option\n", argv[i]);
		exit(1);
	}
}

int inputLang(const char* file) {
	if (inputLanguage) return inputLanguage;
	switch (keyword(intern(ext(file)))) {
	case s_ax:
	case s_p:
		return tptp;
	case s_cnf:
		return dimacs;
	}
	fprintf(stderr, "%s: Unknown file type\n", file);
	exit(1);
}
} // namespace

int main(int argc, const char** argv) {
#ifdef _WIN32
	AddVectoredExceptionHandler(0, handler);
#endif

	// Command line arguments
	parse(argc - 1, argv + 1);
	if (files.empty()) files.add("stdin");

	// If no input file was specified, we default to reading standard input, but that still requires input language to be specified,
	// so with no arguments, just print a usage message
	if (argc <= 1) {
		fprintf(stderr, "Usage: ayane [options] files\n");
		return 1;
	}

	// Attempt problems
	auto file = files[i];
	auto bname = basename(file);

	// Parse
	switch (inputLang(file)) {
	case dimacs:
		parseDimacs(file);
		break;
	case tptp:
		parseTptp(file);
		break;
	}

	// Solve
	auto r = superposn(cs, proof);

	// The SZS ontology uses different result values depending on whether the problem contains a conjecture
	if (problem.hasConjecture) switch (r) {
		case z_Satisfiable:
			r = z_CounterSatisfiable;
			break;
		case z_Unsatisfiable:
			r = z_Theorem;
			break;
		}

	// Print result, and proof if we have one
	printf("%% SZS status %s for %s\n", szsNames[(int)r], bname);
	switch (r) {
	case z_Theorem:
	case z_Unsatisfiable:
		if (proof.count(falsec)) {
			problem.setProof(proofCnf, proof);
			printf("%% SZS output start CNFRefutation for %s\n", bname);
			tptpProof(problem.proofv);
			printf("%% SZS output end CNFRefutation for %s\n", bname);
		}
		break;
	}

	// Print stats
	printStats();
	return 0;
}
