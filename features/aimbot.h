#ifndef AIMBOT_H
#define AIMBOT_H

#include "../players.h"
#include "../utils/history_list.h"

struct Target
{
	int id = -1;
	int backTick = 0;
	int boneID = 0;
	float fov = 420.f;
	int dmg = 0;
	vec3_t targetVec;
};

namespace Aimbot
{
	Target RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer, float oldestTime, bool hitboxList[MAX_HITBOXES]);
}

#endif
