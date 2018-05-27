#include "aimbot.h"

struct Target
{
	int id = -1;
	int backTick = 0;
};

static int ScanHitboxes(HitboxList* list)
{
	return -1;
}

static int LoopPlayers(Players* players, size_t count)
{
	for (size_t i = 0; i < count; i++)
		if (players->flags[i] & Flags::UPDATED)

	return -1;
}

static void FindBestTarget(Target* target, HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer)
{

	float time = localPlayer->time;

	for (size_t i = 0; i < 64; i++) {
		Players& players = track->GetLastItem(i);
		size_t count = players.count;
		int idx = LoopPlayers(&players, count);
		if (idx >= 0) {
			break;
		} else if (idx == -2)
			break;
	}

}

void Aimbot::RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer)
{
	Target target;

	FindBestTarget(&target, tack, localPlayer);

	if (target.id >= 0) {
		vec3_t angles;
		localPlayer->viewangles = angles;
	}
}
