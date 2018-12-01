#ifndef TRACING_H
#define TRACING_H

#include "../players.h"

namespace Tracing
{

	struct Trace
	{
		vec3_t start;
		vec3_t end;
		int entID;
	};

	/*
	  Depth specifies the complexity of the trace to be performed.
	  For example, depth 1 in CSGO would make the trace run through wall penetrating code path,
	  while depth 0 would be a regular traceray.
	*/
	int TracePlayer(LocalPlayer* localPlayer, Players* players, vec3_t point, int eID, int depth = 0, bool skipLocal = true);

	template<size_t N>
	void TracePlayerSIMD(LocalPlayer* localPlayer, Players* players, vec3soa<float, N> point, int eID, int out[N], int depth = 0, bool skipLocal = true);

	void TracePointList(LocalPlayer* localPlayer, Players* players, size_t n, const vec3_t* points, int eIDs, int* __restrict out, int depth = 0, bool skipLocal = true);

	template<size_t N>
	void TracePointListSIMD(LocalPlayer* localPlayer, Players* players, size_t n, const vec3soa<float, N>* points, int eID, int* __restrict out, int depth = 0, bool skipLocal = true);
	/*
	  For games supporting moving players back in time.
	  The mask is to be used for anything the implementation needs it to use (for example marking a Source Engine player as non-backtrackable,
	  due to the breaking of lag compensation)
	*/
	bool BacktrackPlayers(Players* curPlayers, Players* prevPlayers, char backtrackMask[MAX_PLAYERS]);
	bool VerifyTarget(Players* players, int id, char backtrackMask[MAX_PLAYERS]);
}

#endif
