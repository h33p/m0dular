#ifndef CRC32_H
#define CRC32_H

#include <array>

typedef unsigned int crcs_t;

template<typename T>
constexpr auto GenCRCTable(T polynomial = 0xedb88320) {
	constexpr int numBytes = 256;
	constexpr int numIterations = 8;

	std::array<T, numBytes> crc32Table{};

	for (T byte = 0u; byte < numBytes; byte++) {
		T crc = byte;

		for (int i = 0; i < numIterations; i++) {
			T mask = -(crc & 1);
			crc = (crc >> 1) ^ (polynomial & mask);
		}

		crc32Table[byte] = crc;
	}

	return crc32Table;
}

static constexpr auto crc32Tab = GenCRCTable<crcs_t>();

#define CCRC32(x) x##_crc32

#ifdef _WIN32
__forceinline
#else
__inline
__attribute__((always_inline))
__attribute__((no_instrument_function))
#endif
constexpr crcs_t Crc32Helper(crcs_t crc) {
	return ~crc;
}

template<typename T, typename... Args>
#ifdef _WIN32
__forceinline
#else
__inline
__attribute__((always_inline))
__attribute__((no_instrument_function))
#endif
constexpr crcs_t Crc32Helper(crcs_t crc, T arg, Args... args) {
	return Crc32Helper(
		crc32Tab[((crc) ^ (arg)) & 0xff] ^ ((crc) >> 8), args...);
}

template <typename T = char, T...cv>
constexpr crcs_t operator ""_crc32()
{
	return Crc32Helper(~crcs_t(0), cv...);
}

crcs_t Crc32(const char* str, int len);

#endif
