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

int main(int argc, char** argv) {
#ifdef _WIN32
	AddVectoredExceptionHandler(0, handler);
#endif

	// SORT
	const char* file = 0;
	int inputLanguage;
	int outputLanguage;
	///

	// Command line
	for (int i = 1; i < argc; ++i) {
		auto s = argv[i];
		if (*s == '-') {
			do ++s;
			while (*s == '-');
			switch (*s) {
			case 'd':
				inputLanguage = dimacs;
				continue;
			case 'h':
				printf("-h Show help\n"
					   "-V Show version\n"
					   "-d DIMACS input format\n"
					   "-t seconds\n"
					   "   Time limit\n");
				return 0;
			case 't':
			{
				do ++s;
				while (isAlpha(*s));
				switch (*s) {
				case ':':
				case '=':
					++s;
					break;
				case 0:
					++i;
					if (i == argc) {
						fprintf(stderr, "%s: Expected arg\n", argv[i]);
						return 1;
					}
					s = argv[i];
				}
				errno = 0;
				auto seconds = strtod(s, 0);
				if (errno) {
					perror(argv[i]);
					exit(1);
				}
#ifdef _WIN32
				HANDLE timer = 0;
				CreateTimerQueueTimer(&timer, 0, timeout, 0, (DWORD)(seconds * 1000), 0, WT_EXECUTEINTIMERTHREAD);
#else
				alarm(seconds);
#endif
				continue;
			}
			case 'V':
			case 'v':
				printf("Ayane " version);
				if (sizeof(void*) == 4) printf(", 32 bits");
#ifdef DEBUG
				printf(", debug build");
#endif
				putchar('\n');
				return 0;
			case 0:
				// An unadorned '-' means read from standard input, but that's the default anyway if no file is specified, so
				// quietly accept it
				continue;
			}
			fprintf(stderr, "%s: Unknown option\n", argv[i]);
			return 1;
		}
		if (file) {
			fprintf(stderr, "%s: Input file already specified\n", s);
			return 1;
		}
		file = s;
	}

	// Attempt problems
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
