#ifndef AIMBOT_H
#define AIMBOT_H

#include "../players.h"
#include "../utils/history_list.h"

struct AimbotTarget
{
	vec3_t targetVec;
	int id = -1;
	int backTick = 0;
	int boneID = 0;
	float fov = 420.f;
	int dmg = 0;
	bool future = false;
};

namespace Aimbot
{
	AimbotTarget RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, bool hitboxList[MAX_HITBOXES], uint64_t ignoreList[NumOf<64>(MAX_PLAYERS)]);
}

#endif
