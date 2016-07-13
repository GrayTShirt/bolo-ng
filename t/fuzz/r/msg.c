#include <bolo.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	char buf[8192];
	ssize_t n;
	n = read(0, buf, 8192);
	if (n > 0 && n != 8192) {
		size_t left;
		bolo_message_fdump(stdout,
			bolo_message_valid(bolo_message_unpack(buf, n, &left)));
	}
	return 0;
}
