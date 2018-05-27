#ifndef AIMBOT_H
#define AIMBOT_H

#include "../players.h"
#include "../utils/history_list.h"

namespace Aimbot
{
	void RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer);
}

#endif AIMBOT_H
