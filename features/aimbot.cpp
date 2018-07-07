#include "aimbot.h"
#include "../interfaces/tracing.h"

float fovs[64];
int tid = 0;

static vec3_t shootAngles;
static int minDamage = 50;

static bool PreCompareDataLegit(Target* target, LocalPlayer* localPlayer, vec3_t targetVec, int bone, float& outFOV)
{
	vec3_t angle = (targetVec - localPlayer->eyePos).GetAngles(true);
	vec3_t angleDiff = (shootAngles - angle).NormalizeAngles<2>(-180.f, 180.f);
	float fov = angleDiff.Length<2>();
	outFOV = fov;
	return fov < target->fov;
}

static bool CompareDataLegit(Target* target, LocalPlayer* localPlayer, int out, vec3_t targetVec, int bone, float fov)
{

	if (out < minDamage)
		return false;

	if (fov < fovs[tid])
		fovs[tid] = fov;

	if (fov < target->fov) {
		target->boneID = bone;
		target->targetVec = targetVec;
		target->dmg = out;
		target->fov = fov;
		return true;
	}
	return false;
}

static bool CompareDataRage(Target* target, LocalPlayer* localPlayer, int out, vec3_t targetVec, int bone, float fov)
{
	if (out > target->dmg) {
		target->boneID = bone;
		target->targetVec = targetVec;
		target->dmg = out;
		return true;
	}
	return false;
}

bool doMultipoint = true;
float pointScaleVal = 0.8f;

static int ScanHitboxes(Target* target, Players* players, size_t id, LocalPlayer* localPlayer)
{
	fovs[id] = 1000.f;
	tid = id;

	HitboxList& hitboxes = players->hitboxes[id];

	for (size_t i = 0; i < MAX_HITBOXES; i++) {

		float fov = 0.f;

		vec3_t average = (hitboxes.start[i] + hitboxes.end[i]) * 0.5f;
		average = hitboxes.wm[i].Vector3Transform(average);

		if (true && !PreCompareDataLegit(target, localPlayer, average, i, fov))
			continue;

		//TODO Finish multipoint
		if (doMultipoint) {
			bool quit = false;

			int out[MULTIPOINT_COUNT];
			mvec3 mpVec = players->hitboxes[id].mpOffset[i] + players->hitboxes[id].mpDir[i] * players->hitboxes[id].radius[i] * pointScaleVal;
			mpVec = players->hitboxes[id].wm[i].VecSoaTransform(mpVec);
			Tracing::TracePlayersSIMD<MULTIPOINT_COUNT>(localPlayer, players, mpVec, id, out);

			for (size_t o = 0; o < MULTIPOINT_COUNT; o++) {
				if (true && CompareDataLegit(target, localPlayer, out[o], (vec3_t)mpVec.acc[o], i, fov))
					quit = true;
				if (false && CompareDataRage(target, localPlayer, out[o], (vec3_t)mpVec.acc[o], i, fov))
					quit = true;
			}

			if (quit)
				return 1;
		} else {

			int out = Tracing::TracePlayers(localPlayer, players, average, id);

			if (true && CompareDataLegit(target, localPlayer, out, average, i, fov))
				return 1;
			if (false && CompareDataRage(target, localPlayer, out, average, i, fov))
				return 1;
		}
	}

	return 0;
}

static int LoopPlayers(Target* target, Players* players, size_t count, LocalPlayer* localPlayer, float oldestTime)
{
	int ret = 0;

	for (size_t i = 0; i < count; i++) {
		if (players->flags[i] & Flags::HITBOXES_UPDATED && ~players->flags[i] & Flags::FRIENDLY && players->fov[i] - 4.f < target->fov) {
			if (ScanHitboxes(target, players, i, localPlayer)) {
				target->id = i;
				ret = 1;
			}
		}
	}

    return ret;
}

static void FindBestTarget(Target* target, HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer, float oldestTime)
{

	char backtrackMask[MAX_PLAYERS];
	float lowestFov = 1000.f;
	Players* prevPlayers = nullptr;

	memset(backtrackMask, 0, MAX_PLAYERS);

	for (size_t i = 0; i < BACKTRACK_TICKS; i++) {
		Players& players = track->GetLastItem(i);
		size_t count = players.count;
		Target t2;

		if (!Tracing::BacktrackPlayers(&players, prevPlayers, backtrackMask))
			break;

		LoopPlayers(&t2, &players, count, localPlayer, oldestTime);
		if (t2.id >= 0 && t2.fov < lowestFov) {
			lowestFov = t2.fov;
			t2.backTick = i;
			*target = t2;
		}
		prevPlayers = &players;
	}
}

Target Aimbot::RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer, float oldestTime)
{
	Target target;
	shootAngles = localPlayer->angles + localPlayer->aimOffset;

	FindBestTarget(&target, track, localPlayer, oldestTime);

	if (target.id >= 0) {
		vec3_t angles = (target.targetVec - localPlayer->eyePos).GetAngles(true);
		localPlayer->angles = angles - localPlayer->aimOffset;
	}

	return target;
}
