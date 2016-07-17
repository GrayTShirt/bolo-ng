#include <ring.h>
#include <string.h>

int main(int argc, char **argv)
{
	ring_t r;
	ssize_t n;
	char buf[256];

#define NEW(c) do { \
	r = ring_alloc(c); \
	if (!r) { \
		printf("ERROR: failed to allocate a ring(%d)", c); \
		return 1; \
	} \
} while (0)

#define FREE() do { \
	ring_free(r); \
	r = NULL; \
} while (0)

#define FILL(s) do { \
	n = ring_fill(r, s, strlen(s)); \
	if (n < 0) { \
		printf("ERROR: failed to fill '%s' (%lu)\n", s, strlen(s)); \
		return 1; \
	} \
	ring_fdump(stdout, r); \
} while (0)

#define COPY(c) do { \
	n = ring_copy(buf, r, c); \
	if (n < 0) { \
		printf("ERROR: failed to copy %i bytes\n", c); \
		return 1; \
	} \
	ring_fdump(stdout, r); \
} while (0)

#define MOVE(c) do { \
	n = ring_move(buf, r, c); \
	if (n < 0) { \
		printf("ERROR: failed to move %i bytes\n", c); \
		return 1; \
	} \
	ring_fdump(stdout, r); \
} while (0)

	NEW(256);
		FILL("Hello, World!");
		FILL(" (some more data)");
		MOVE(5);
		MOVE(254);
		FILL("extra datas");
		COPY(0);
		MOVE(4);
		COPY(100);
	FREE();

	NEW(8192);
	FREE();

	return 0;
}
