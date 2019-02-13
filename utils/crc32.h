#ifndef CRC32_H
#define CRC32_H

#include <array>
#include "shared_utils.h"

typedef unsigned int crcs_t;

template<typename T>
constexpr auto GenCRCTable(T polynomial = 0xedb88320) {
	constexpr int numBytes = 256;
	constexpr int numIterations = 8;

	std::array<T, numBytes> crc32Table{};

	for (T byte = 0u; byte < numBytes; byte++) {
		T crc = byte;

		for (int i = 0; i < numIterations; i++) {
			//Can be done with -(crc & 1), but then MSVC complains about overflows
			T mask = (crc & 1) ? ~T(0) : T(0);
			crc = (crc >> 1) ^ (polynomial & mask);
		}

		crc32Table[byte] = crc;
	}

	return crc32Table;
}

static constexpr auto crc32Tab = GenCRCTable<crcs_t>();

#define CCRC32(x) calc_constexpr<crcs_t, ConstantCrc32(x, sizeof(x) - 1)>::value

constexpr crcs_t ConstantCrc32(const char* cv, size_t size)
{
	crcs_t ret(0);
	ret = ~ret;

	for (size_t i = 0; i < size; i++)
		ret = crc32Tab[((ret) ^ (cv[i])) & 0xff] ^ ((ret) >> 8);

	return ~ret;
}

constexpr crcs_t operator ""_crc32(const char* cv, size_t size)
{
	return ConstantCrc32(cv, size);
}

crcs_t Crc32(const char* str, int len);
crcs_t Crc32(const char* str);

#endif
