#ifndef TRACING_H
#define TRACING_H

#include "../players.h"

namespace Tracing
{
	int TracePlayers(LocalPlayer* localPlayer, Players* players, vec3_t point, bool skipLocal = true);
	void TracePlayersSIMD(LocalPlayer* localPlayer, Players* players, nvec3 point, int out[SIMD_COUNT], bool skipLocal = true);

	/*
	  For games supporting moving players back in time.
	  The mask is to be used for anything the implementation needs it to use (for example marking a Source Engine player as non-backtrackable,
	  due to the breaking of lag compensation)
	*/
	bool BacktrackPlayers(Players* curPlayers, Players* prevPlayers, char backtrackMask[MAX_PLAYERS]);
}

#endif
