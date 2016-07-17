#include <ring.h>
#include <string.h>

int main(int argc, char **argv)
{
	char buf[16], *p;
	int i, c;
	size_t n;
	ssize_t l;
	ring_t r;

	r = ring_alloc(16);
	if (!r) {
		printf("ERROR: failed to allocate a ring(16)\n");
		return 1;
	}

	ring_fdump(stdout, r);
	for (i = 1; i < argc; i++) {
		n = strlen(argv[i]) - 1;
		if (argv[i][0] == '+') {
			l = ring_fill(r, argv[i]+1, n);
			if (l < 0) {
				printf("failed to fill ring with %zu bytes of data\n", n);
				return 0; /* sometimes that's ok */
			}
		} else if (argv[i][0] == '-') {
			l = ring_move(buf, r, argv[i][1]);
			if (l < 0) {
				printf("failed to move %d bytes from ring\n", argv[i][1]);
				return 0; /* sometimes that's ok */
			}
			printf("buf [%s]\n", buf);
		} else {
			printf("ERROR: bad argument '%s' (should start '+' or '-')\n", argv[i]);
			return 0;
		}

		ring_fdump(stdout, r);
	}

	ring_free(r);
	return 0;
}
