#include "main.h"

#include <new>

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
	ExitProcess(1);
}

VOID CALLBACK timeout(PVOID a, BOOLEAN b) {
	// On Linux the exit code associated with timeout is 128+SIGALRM. On Windows, this exit code serves as well as any, and means a
	// script that calls the program can use the same code to check for timeout on both platforms.
	ExitProcess(128 + 14);
}
} // namespace
#else
#include <unistd.h>
#endif

#define version "3"

enum
{
	cxx = 1,
	dimacs,
	tptp,
};

namespace {
const char* extension(const char* file) {
	auto s = strrchr(file, '.');
	return s ? s + 1 : "";
}

struct listParser: parser {
	listParser(const char* file, vec<const char*>& v): parser(file) {
		for (;;) {
			// Find start of next line, skipping blanks.
			while (isSpace(*src)) ++src;

			// Null terminator indicates end of file.
			if (!*src) break;

			// Find end of line.
			auto s = src;
			while (isPrint(*s)) ++s;

			// Store a copy of the line.
			auto n = s - src;
			auto r = new char[n + 1];
			memcpy(r, src, n);
			r[n] = 0;
			v.push_back(r);

			// Continue.
			src = s;
		}
	}
};

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
bool cnfOnly;
std::vector<const char*> files;
int inputLanguage;
uint64_t iterLimit = ~(uint64_t)0;
size_t memLimit = (size_t)1 << (sizeof(void*) == 4 ? 30 : 31);
int outputLanguage;
///

void parse(int argc, const char** argv) {
	for (int i = 0; i != argc; ++i) {
		auto s = argv[i];

		// File.
		if (*s != '-') {
			if (!strcmp(extension(s), "lst")) {
				vec<const char*> v;
				listParser p(s, v);
				parse(v.size(), v.data());
				continue;
			}
			files.push_back(s);
			continue;
		}

		// Option.
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
		// accept it.
		if (!bufi) continue;

		// Option.
		switch (keyword(intern(buf, bufi))) {
		case s_C:
			iterLimit = optDouble(argc, argv, i, oa);
			continue;
		case s_cnf:
			cnfOnly = 1;
			continue;
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
		case s_cxx:
			inputLanguage = cxx;
			continue;
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
		case s_question:
			printf(
				// SORT
				"-C count      Max iterations of main superposition loop\n"
				"-cnf          Convert problem to clause normal form\n"
				"-dimacs       Set DIMACS as input and output format\n"
				"-dimacs-in    Set DIMACS as input format\n"
				"-dimacs-out   Set DIMACS as output format\n"
				"-h            Show help\n"
				"-m megabytes  Memory limit (default %zu)\n"
				"-t seconds    Time limit\n"
				"-tptp         Set TPTP as input and output format\n"
				"-tptp-in      Set TPTP as input format\n"
				"-tptp-out     Set TPTP as output format\n"
				"-V            Show version\n"
				///
				,
				memLimit / (1 << 20));
			exit(0);
		case s_in:
			continue;
		case s_m:
		case s_memory:
		case s_memorylimit:
			memLimit = optDouble(argc, argv, i, oa) * (1 << 20);
			continue;
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
			printf(
				"Ayane version " version ", %zu-bit "
#ifdef DEBUG
				"debug"
#else
				"release"
#endif
				" build\n",
				sizeof(void*) * 8);
			exit(0);
		}
		fprintf(stderr, "%s: Unknown option\n", argv[i]);
		exit(1);
	}
}

int inputLang(const char* file) {
	if (inputLanguage) return inputLanguage;
	switch (keyword(intern(extension(file)))) {
	case s_ax:
	case s_p:
		return tptp;
	case s_cc:
	case s_cpp:
		return cxx;
	case s_cnf:
		return dimacs;
	}
	fprintf(stderr, "%s: Unknown file type\n", file);
	exit(1);
}
} // namespace

int main(int argc, const char** argv) {
	std::set_new_handler([]() {
		perror("new");
		exit(1);
	});
#ifdef _WIN32
	AddVectoredExceptionHandler(0, handler);
#endif
	initBignums();

	// Run unit tests, if this is a debug build.
	test();

	// Command line arguments.
	parse(argc - 1, argv + 1);
	if (files.empty()) files.push_back("stdin");

	// If no input file was specified, we default to reading standard input, but that still requires input language to be specified,
	// so with no arguments, just print a usage message.
	if (argc <= 1) {
		fprintf(stderr, "Usage: ayane [options] files\n");
		return 1;
	}

	// Attempt problems.
	for (size_t i = 0; i != files.size(); ++i) {
		auto file = files[i];
		auto bname = basename(file);

		// Initialize.
		clearStrings();

		// Parse.
		Problem problem;
		switch (inputLang(file)) {
		case dimacs:
			parseDimacs(file, problem);
			break;
		case tptp:
			parseTptp(file, problem);
			break;
		}

		// Gather up the input formulas. The parser places them in a map that also indicates sources, but we need to collect them
		// into a simple set of formulas for the next step.
		set<term> initialFormulas;
		for (auto& p: problem.initialFormulas) initialFormulas.add(p.first);

		// Convert to CNF.
		ProofCnf proofCnf;
		set<clause> cs;
		cnf(initialFormulas, proofCnf, cs);
		if (cnfOnly) {
			size_t id = 0;
			for (auto& c: cs) tptpClause(c, ++id);
			return 0;
		}

		// Solve.
		Proof proof;
		auto r = superposn(cs, proof, iterLimit);

		// The SZS ontology uses different result values depending on whether the problem contains a conjecture.
		if (problem.hasConjecture) switch (r) {
			case szs::Satisfiable:
				r = szs::CounterSatisfiable;
				break;
			case szs::Unsatisfiable:
				r = szs::Theorem;
				break;
			}

		// Print result, and proof if we have one.
		printf("%% SZS status %s for %s\n", szsNames[(int)r], bname);
		switch (r) {
		case szs::Theorem:
		case szs::Unsatisfiable:
			if (proof.count(falsec)) {
				problem.setProof(proofCnf, proof);
				printf("%% SZS output start CNFRefutation for %s\n", bname);
				tptpProof(problem.proofv);
				printf("%% SZS output end CNFRefutation for %s\n", bname);
			}
			break;
		}

		// Print stats.
		printStats();
		if (files.size() > 1) putchar('\n');
	}
	return 0;
}
