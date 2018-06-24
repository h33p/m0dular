#ifndef MD5_H
#define MD5_H

constexpr int MD5_DIGEST_LENGTH = 16;

typedef struct
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
} MD5Context_t;

namespace MD5 {
	void Init(MD5Context_t* context);
	void Update(MD5Context_t* context, unsigned char const* buf, unsigned int len);
	void Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t* context);
	unsigned int PseudoRandom(unsigned int nSeed);
}

#endif
