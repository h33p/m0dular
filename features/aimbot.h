#ifndef AIMBOT_H
#define AIMBOT_H

#include "../players.h"
#include "../utils/history_list.h"

#include "aimbot_types.h"

namespace Aimbot
{
	extern vec3_t shootAngles;

	AimbotTarget RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, unsigned char hitboxList[MAX_HITBOXES], uint64_t ignoreList[NumOf<64>(MAX_PLAYERS)], float pointScale[MAX_PLAYERS]);

	//These need to be implemented manually
	bool PreCompareData(AimbotTarget* target, LocalPlayer* localPlayer, vec3_t targetVec, int bone, float* outFOV);
	bool CompareData(AimbotLoopData* d, int out, vec3_t targetVec, int bone, float fov);
}

#endif
