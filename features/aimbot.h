#ifndef AIMBOT_H
#define AIMBOT_H

#include "../players.h"
#include "../utils/history_list.h"

#include "aimbot_types.h"

namespace Aimbot
{
	AimbotTarget RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, unsigned char hitboxList[MAX_HITBOXES], uint64_t ignoreList[NumOf<64>(MAX_PLAYERS)], float pointScale[MAX_PLAYERS]);
}

#endif
