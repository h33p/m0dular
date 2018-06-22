#ifndef TRACING_H
#define TRACING_H

#include "../players.h"

namespace Tracing
{
	int TracePlayers(LocalPlayer* localPlayer, Players* players, vec3_t point, bool skipLocal = true);
	void TracePlayersSIMD(LocalPlayer* localPlayer, Players* players, nvec3 point, int out[SIMD_COUNT], bool skipLocal = true);
}

#endif
