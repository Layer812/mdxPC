#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include "tools.h"

// mdxCP/ pdx file load on flash / Layer8

int gcd(int a, int b) {
	int c = a % b;

	while(c > 0) {
		a = b;
		b = c;
		c = a % b;
	}

	return b;
}



void csv_quote(char *str, size_t len) {
	if(len == 0) len = strlen(str);

	if(str == 0) {
		putchar('\\');
		putchar('N');
		return;
	}

	putchar('"');
	for(int i = 0; i < len; i++) {
		switch(str[i]) {
			case 0:
				putchar('\\');
				putchar(0);
				break;
			case '\\':
				putchar('\\');
				putchar('\\');
				break;
			case '\b':
				putchar('\\');
				putchar('b');
				break;
			case '\n':
				putchar('\\');
				putchar('n');
				break;
			case '\r':
				putchar('\\');
				putchar('r');
				break;
			case '\t':
				putchar('\\');
				putchar('t');
				break;
			case 26:
				putchar('\\');
				putchar('Z');
				break;
			case '"':
				putchar('"');
				putchar('"');
				break;
			default: putchar(str[i]);
		}
	}
	putchar('"');
}

void hex_dump(const uint8_t *data, size_t len) {
	for(size_t i = 0; i < len; i++) printf("%02x", data[i]);
}

int replace_ext(char *out, size_t out_size, const char *in, const char *ext) {
	int inlen = strlen(in);
	int extlen = strlen(ext);
	if(out_size < inlen + extlen + 2)
		return -1;
	strncpy(out, in, out_size);
	char *b = basename(out);
	char *pp = strrchr(b, '.');
	if(!pp)
		pp = b + strlen(b);
	if(pp - out + extlen + 1 > out_size)
		extlen = out_size - (pp - out + 1);
	if(extlen > 0)
		strncpy(pp + 1, ext, extlen+1);

	return 0;
}
