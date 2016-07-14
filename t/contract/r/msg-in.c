#include <bolo.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char buf[8192];
	ssize_t n;
	n = read(0, buf, 8192);
	if (n > 0 && n != 8192) {
		size_t left;
		bolo_message_t m;

		m = bolo_message_unpack(buf, n, &left);
		if (!bolo_message_valid(m)) {
			printf("~~ BOGON DETECTED ~~\n");
		}
		bolo_message_fdump(stdout, m);
		return 0;
	}
	return 1;
}
