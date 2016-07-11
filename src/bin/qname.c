#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bolo.h>

void chomp(char *s)
{
	char *nl = strrchr(s, '\n');
	if (nl) {
		*nl = '\0';
	}
}

int main(int argc, char **argv)
{
	char *s, buf[8192];
	while ( (fgets(buf, 8192, stdin)) != NULL ) {
		chomp(buf);
		bolo_qname_t qn = bolo_qname_parse(buf);
		s = bolo_qname_string(qn);
		printf("%s\n", s);
		free(s);
		bolo_qname_free(qn);
	}
	return 0;
}
