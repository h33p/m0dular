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

enum HitboxScanMode_t : unsigned char
{
	SCAN_NONE = 0,
	SCAN_SIMPLE = 1,
	SCAN_MULTIPOINT = 2
};

namespace Aimbot
{
	AimbotTarget RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, unsigned char hitboxList[MAX_HITBOXES], uint64_t ignoreList[NumOf<64>(MAX_PLAYERS)]);
}

#endif
