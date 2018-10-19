#include "aimbot.h"
#include "../interfaces/tracing.h"

float fovs[64];
int tid = 0;

static vec3_t shootAngles;
static int minDamage = 50;

static bool PreCompareDataLegit(AimbotTarget* target, LocalPlayer* localPlayer, vec3_t targetVec, int bone, float& outFOV)
{
	vec3_t angle = (targetVec - localPlayer->eyePos).GetAngles(true);
	vec3_t angleDiff = (shootAngles - angle).NormalizeAngles<2>(-180.f, 180.f);
	float fov = angleDiff.Length<2>();
	outFOV = fov;
	return fov < target->fov;
}

static bool CompareDataLegit(AimbotTarget* target, LocalPlayer* localPlayer, int out, vec3_t targetVec, int bone, float fov)
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

static bool CompareDataRage(AimbotTarget* target, LocalPlayer* localPlayer, int out, vec3_t targetVec, int bone, float fov)
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
float pointScaleVal = 0.98f;

static int ScanHitboxes(AimbotTarget* target, Players* players, size_t id, LocalPlayer* localPlayer, bool hitboxList[MAX_HITBOXES])
{
	fovs[id] = 1000.f;
	tid = id;

	HitboxList& hitboxes = players->hitboxes[id];

	for (size_t i = 0; i < MAX_HITBOXES; i++) {

		if (!hitboxList[i])
			continue;

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

static int LoopPlayers(AimbotTarget* target, Players* players, size_t count, LocalPlayer* localPlayer, bool hitboxList[MAX_HITBOXES])
{
	int ret = 0;

	for (size_t i = 0; i < count; i++) {
		if (players->flags[i] & Flags::HITBOXES_UPDATED && ~players->flags[i] & Flags::FRIENDLY && players->fov[i] - 4.f < target->fov) {
			if (ScanHitboxes(target, players, i, localPlayer, hitboxList)) {
				target->id = i;
				ret = 1;
			}
		}
	}

    return ret;
}

static void FindBestTarget(AimbotTarget* target, HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, bool hitboxList[MAX_HITBOXES])
{

	char backtrackMask[MAX_PLAYERS];
	float lowestFov = 1000.f;
	Players* prevPlayers = nullptr;
	Players* targetPlayers = nullptr;

	memset(backtrackMask, 0, MAX_PLAYERS);

	//First check the future, but this will be overwritten by the normal track if any of the ticks are valid
	if (futureTrack) {
		for (size_t i = 0; i < futureTrack->Count(); i++) {
			Players& players = futureTrack->GetLastItem(i);
			size_t count = players.count;
			AimbotTarget t2;

			//We do not want to just exit out the loop if we predicted too far into future
			if (!Tracing::BacktrackPlayers(&players, prevPlayers, backtrackMask))
				continue;

			LoopPlayers(&t2, &players, count, localPlayer, hitboxList);
			if (t2.id >= 0 && t2.fov < lowestFov) {
				lowestFov = t2.fov;
				t2.backTick = i;
				t2.future = true;
				*target = t2;
				targetPlayers = &players;
			}
			prevPlayers = &players;
		}
	}

	for (size_t i = 0; i < track->Count(); i++) {
		Players& players = track->GetLastItem(i);
		size_t count = players.count;
		AimbotTarget t2;

		if (!Tracing::BacktrackPlayers(&players, prevPlayers, backtrackMask))
			break;

		LoopPlayers(&t2, &players, count, localPlayer, hitboxList);
		if (t2.id >= 0 && (t2.fov < lowestFov || !Tracing::VerifyTarget(targetPlayers, target->id, backtrackMask))) {
			lowestFov = t2.fov;
			t2.backTick = i;
			t2.future = false;
			*target = t2;
			targetPlayers = &players;
		}
		prevPlayers = &players;
	}
}

AimbotTarget Aimbot::RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, bool hitboxList[MAX_HITBOXES])
{
	AimbotTarget target;
	shootAngles = localPlayer->angles + localPlayer->aimOffset;

	FindBestTarget(&target, track, futureTrack, localPlayer, hitboxList);

	if (target.id >= 0) {
		vec3_t angles = (target.targetVec - localPlayer->eyePos).GetAngles(true);
		localPlayer->angles = angles - localPlayer->aimOffset;
	}

	return target;
}
