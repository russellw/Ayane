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
	// TODO: use the same exit code as on Linux
	ExitProcess(7);
}
} // namespace
#else
#include <unistd.h>
#endif

#define version "3"

enum class Language {
	dimacs,
	none,
	tptp,
};

static const char* ext(const char* file) {
	auto s = strrchr(file, '.');
	return s ? s + 1 : "";
}

int main(int argc, char** argv) {
#ifdef _WIN32
	AddVectoredExceptionHandler(0, handler);
#endif

	// Command line
	auto language = Language::none;
	char* file = 0;
	for (int i = 1; i < argc; ++i) {
		auto s = argv[i];
		if (*s == '-') {
			do ++s;
			while (*s == '-');

			switch (*s) {
			case 'd':
				language = Language::dimacs;
				continue;
			case 'h':
				printf("-h Show help\n"
					   "-V Show version\n"
					   "-d DIMACS input\n"
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
					break;
				}

				errno = 0;
				auto seconds = strtod(s, 0);
				if (errno) {
					perror(argv[i]);
					return 1;
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
#ifdef DBG
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
	if (!file) file = "stdin";
	if (language == Language::none) switch (keyword(intern(ext(file)))) {
		case s_cnf:
			language = Language::dimacs;
			break;
		}

	// Parse
	switch (language) {
	case Language::dimacs:
		dimacs(file);
		break;
	case Language::tptp:
		tptp(file);
		break;
	}

	// Solve
	superposn();

	// Print result
	switch (result) {
	case 0:
		puts("unsat");
		break;
	case 1:
		puts("sat");
		break;
	}

	// Print stats
	printStats();
	return 0;
}
