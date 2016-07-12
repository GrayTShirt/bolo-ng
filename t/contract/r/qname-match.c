#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bolo.h>

int main(int argc, char **argv)
{
	bolo_qname_t a, b;
	char mode;
	int rc;

	if (argc != 4) {
		fprintf(stderr, "incorrect usage.  try %s a=b '~' c=d\n", argv[1]);
		return 2;
	}

	a = bolo_qname_parse(argv[1]);
	mode = argv[2][0];
	b = bolo_qname_parse(argv[3]);
	rc = bolo_qname_match(a,b);
	bolo_qname_free(a);
	bolo_qname_free(b);

	switch (mode) {
	case '~': return (rc ? 0 : 1);
	case '!': return (rc ? 1 : 0);
	}
	fprintf(stderr, "incorrect usage.  comparison must be '~' or '!' (not '%c')\n", argv[2][0]);
	return 2;
}
