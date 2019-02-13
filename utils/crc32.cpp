#include "crc32.h"

crcs_t Rcrc32Helper(const char* v, unsigned int c,
	crcs_t crc) {
	return c == 0 ?
		~crc :
		Rcrc32Helper(&v[1], c - 1,
			crc32Tab[((crc) ^ (v[0])) & 0xff] ^ ((crc) >> 8));
}

crcs_t Rcrc32Helper(const char* v, crcs_t crc) {
	return !*v ?
		~crc :
		Rcrc32Helper(v + 1, crc32Tab[((crc) ^ (*v)) & 0xff] ^ ((crc) >> 8));
}

crcs_t Crc32(const char* str, int len)
{
	return Rcrc32Helper(str, len, ~crcs_t(0));
}

crcs_t Crc32(const char* str)
{
	return Rcrc32Helper(str, ~crcs_t(0));
}
