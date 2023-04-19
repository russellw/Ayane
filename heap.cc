#include "main.h"

#ifdef _WIN32
#include <windows.h>

[[noreturn]] void werr(const char* prefix) {
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf,
		sizeof buf,
		0);
	fprintf(stderr, "%s: %s", prefix, buf);
	exit(1);
}

void* reserve(size_t n) {
	// TODO: can we use big pages?
	auto p = VirtualAlloc(0, n, MEM_RESERVE, PAGE_READWRITE);
	if (!p) werr("VirtualAlloc");
	return p;
}

void commit(void* p, size_t n) {
	p = VirtualAlloc(p, n, MEM_COMMIT, PAGE_READWRITE);
	if (!p) werr("VirtualAlloc");
}
#else
#include <sys/mman.h>

void* reserve(size_t n) {
	// On a system with overcommit turned on, a simple malloc would suffice. However, overcommit is not always turned on. For
	// example, on WSL it is off by default.
	// TODO: can we use big pages?
	auto p = mmap(0, n, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!p) {
		perror("mmap");
		exit(1);
	}
	return p;
}

void commit(void* p, size_t n) {
	// This assumes slab size is at least as big as page size, so the pointer is already aligned.
	if (mprotect(p, n, PROT_READ | PROT_WRITE)) {
		perror("mprotect");
		exit(1);
	}
}
#endif

Heap<>* heap;

namespace {
struct init {
	init() {
		heap = Heap<>::make();
	}
} _;
} // namespace
