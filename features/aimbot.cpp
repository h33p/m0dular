#include "aimbot.h"
#include "../interfaces/tracing.h"

float fovs[64];
int tid = 0;

static bool CompareDataLegit(Target* target, LocalPlayer* localPlayer, int out, vec3_t targetVec, int bone)
{
	vec3_t angle = (targetVec - localPlayer->eyePos).GetAngles(true);
	vec3_t angleDiff = (localPlayer->angles - angle).NormalizeAngles<2>(-180.f, 180.f);
	float fov = angleDiff.Length<2>();
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

static bool CompareDataRage(Target* target, LocalPlayer* localPlayer, int out, vec3_t targetVec, int bone)
{
	if (out > target->dmg) {
		target->boneID = bone;
		target->targetVec = targetVec;
		target->dmg = out;
		return true;
	}
	return false;
}

static int ScanHitboxes(Target* target, Players* players, size_t id, LocalPlayer* localPlayer)
{
	fovs[id] = 1000.f;
	tid = id;

	HitboxList& hitboxes = players->hitboxes[id];

	for (size_t i = 0; i < MAX_HITBOXES; i++) {

		//TODO Multipoint
		if (false) {
			/*bool quit = false;

			int out[SIMD_COUNT];
			nvec3 average = (players->hitboxes[id].start[i] + players->hitboxes[id].end[i]) * 0.5f;
			average.TransformInPlace(players->hitboxes[id].wm + SIMD_COUNT * i);
			Tracing::TracePlayersSIMD(localPlayer, players, average, out);

			nvec3 angles = average.GetAngles();

			for (size_t o = 0; o < SIMD_COUNT; o++) {
				if (out[i] > target->dmg) {
					target->boneID = i * SIMD_COUNT + o;
					quit = true;
				}
			}

			if (quit)
			return 1;*/
		} else {

			vec3_t average = (hitboxes.start[i] + hitboxes.end[i]) * 0.5f;
			average = hitboxes.wm[i].Vector3Transform(average);
			int out = Tracing::TracePlayers(localPlayer, players, average);

			if (true && CompareDataLegit(target, localPlayer, out, average, i))
				return 1;
			if (false && CompareDataRage(target, localPlayer, out, average, i))
			    return 1;
		}
	}

    return 0;
}

static int LoopPlayers(Target* target, Players* players, size_t count, LocalPlayer* localPlayer, float oldestTime)
{
	for (size_t i = 0; i < count; i++) {
		if (players->flags[i] & Flags::HITBOXES_UPDATED && players->time[i] < oldestTime) {
			target->id = -2;
			return 0;
		}
	}

	int ret = 0;

	for (size_t i = 0; i < count; i++) {
		if (players->flags[i] & Flags::HITBOXES_UPDATED && players->fov[i] - 10.f < target->fov) {
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

	float lowestFov = 1000.f;

	for (size_t i = 0; i < BACKTRACK_TICKS; i++) {
		Players& players = track->GetLastItem(i);
		size_t count = players.count;
		Target t2;
		LoopPlayers(&t2, &players, count, localPlayer, oldestTime);
		if (t2.id >= 0 && t2.fov < lowestFov) {
			lowestFov = t2.fov;
			t2.backTick = i;
			*target = t2;
		} else if (t2.id == -2)
			break;
	}
}

Target Aimbot::RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, LocalPlayer* localPlayer, float oldestTime)
{
	Target target;

	FindBestTarget(&target, track, localPlayer, oldestTime);

	if (target.id >= 0) {
		vec3_t angles = (target.targetVec - localPlayer->eyePos).GetAngles(true);
		localPlayer->angles = angles;
	}

	return target;
}
