#ifndef PATTERN_SCAN_H
#define PATTERN_SCAN_H

#include "stdint.h"

namespace PatternScan
{
	uintptr_t FindPattern(const char* pattern, uintptr_t start, uintptr_t end);
}

#endif
