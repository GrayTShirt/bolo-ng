#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bolo.h>

char* chomp(char *s)
{
	char *nl = strrchr(s, '\n');
	if (nl) {
		*nl = '\0';
	}
	return s;
}

int main(int argc, char **argv)
{
	char buf[8192];
	while ( (fgets(buf, 8192, stdin)) != NULL ) {
		printf("%s\n", bolo_qname_string(bolo_qname_parse(chomp(buf))));
	}
	return 0;
}
