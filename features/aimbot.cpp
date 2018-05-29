#include "aimbot.h"
#include "../interfaces/tracing.h"

struct Target
{
	int id = -1;
	int backTick = 0;
	int boneID = 0;
	float fov;
};

static void ScanHitboxes(Target* target, Players* players, size_t id, LocalPlayer* localPlayer)
{

	for (size_t i = 0; i < HITBOX_CHUNKS; i++) {
		int out[SIMD_COUNT];
		nvec3 average = (players->hitboxes[i].start[i] + players->hitboxes[i].end[i]) * 0.5f;
		Tracing::TracePlayersSIMD(localPlayer, players, average, out);
	}

	target->boneID = -1;
}

static void LoopPlayers(Target* target, Players* players, size_t count, LocalPlayer* localPlayer, float oldestTime)
{
	for (size_t i = 0; i < count; i++) {
		if (players->flags[i] & Flags::UPDATED && players->time[i] < oldestTime) {
			target->id = -2;
			return;
		}
	}

	for (size_t i = 0; i < count; i++) {
		if (players->flags[i] & Flags::UPDATED) {
			ScanHitboxes(target, players, i, localPlayer);
			if (target->boneID) {
				target->id = i;
				return;
			}
		}
	}

	target->id = -1;
}

static void FindBestTarget(Target* target, HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer, float oldestTime)
{

	float lowestFov = 1000.f;

	for (size_t i = 0; i < BACKTRACK_TICKS; i++) {
		Players& players = track->GetLastItem(i);
		size_t count = players.count;
		Target t2;
		LoopPlayers(&t2, &players, count, localPlayer, oldestTime);
		if (t2.id >= 0 && t2.fov < lowestFov) {
			lowestFov = t2.fov;
			*target = t2;
		} else if (t2.id == -2)
			break;
	}
}

void Aimbot::RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer, float oldestTime)
{
	Target target;

	FindBestTarget(&target, track, localPlayer, oldestTime);

	if (target.id >= 0) {
		vec3_t angles;
		localPlayer->angles = angles;
	}
}
