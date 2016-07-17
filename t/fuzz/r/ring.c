#include <ring.h>
#include <string.h>

int main(int argc, char **argv)
{
	char buf[4096], *p;
	int i;
	size_t n;
	ssize_t l;
	ring_t r;

	r = ring_alloc(8192);
	if (!r) {
		return 1;
	}

	while ( (fgets(buf, 4096, stdin)) != NULL ) {
		n = strlen(buf);
		if (n < 1) {
			continue;
		}

		ring_fill(r, buf, n);
		n = buf[0];
		ring_copy(buf, r, n);
		ring_copy(buf, r, n);
	}

	ring_free(r);
	return 0;
}
