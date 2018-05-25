#include "crc32.h"

unsigned int Rcrc32Helper(char* v, unsigned int c,
	unsigned int crc) {
	return c == 0 ?
		~crc :
		Rcrc32Helper(&v[1], c - 1,
			crc32Tab[((crc) ^ (v[0])) & 0xff] ^ ((crc) >> 8));
}

unsigned int Crc32(char* str, int len)
{
	return Rcrc32Helper(str, len, 0xFFFFFFFF);
}
