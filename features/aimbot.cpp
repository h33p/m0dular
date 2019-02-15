#include "aimbot.h"
#include "../interfaces/tracing.h"
#ifdef AIMBOT_THREADING
#include "../utils/threading.h"
auto& numThreads = Threading::numThreads;
constexpr int threadQueueMultiplier = 2;
#else
constexpr int numThreads = 1;
constexpr int threadQueueMultiplier = 1;
#endif

vec3_t Aimbot::shootAngles;
float* pointScaleVal = nullptr;

#ifdef AIMBOT_THREADING
static Semaphore threadSem;
#endif

AimbotLoopData data[NUM_THREADS * 2];

bool doMultipoint = true;

static int ProcessAimPointsSIMD(AimbotLoopData* d)
{
	int ret = -1;

#ifdef AIMBOT_SIMD_TRACE_DATA
	Tracing::TracePointListSIMD<MULTIPOINT_COUNT>(d->localPlayer, d->players, d->hitboxIDsSOA.size(), d->traceEndSOA.data(), d->entID, d->traceOutputsSOA.data(), 1);

	for (size_t i = 0; i < d->hitboxIDsSOA.size(); i++) {
		bool quit = false;

		for (size_t o = 0; o < MULTIPOINT_COUNT; o++)
			if (Aimbot::CompareData(d, d->traceOutputsSOA[i * MULTIPOINT_COUNT + o], (vec3_t)d->traceEndSOA[i].acc[o], d->hitboxIDsSOA[i], d->fovListSOA[i * MULTIPOINT_COUNT + o]))
				quit = true;

		//TODO: add an option to return early
		if (quit)
			ret = d->entID;
	}

#endif
	return ret;
}

static int ProcessAimPoints(AimbotLoopData* d)
{
	Tracing::TracePointList(d->localPlayer, d->players, d->hitboxIDs.size(), d->traceEnd.data(), d->entID, d->traceOutputs.data(), 1);

	int ret = -1;

	for (size_t i = 0; i < d->hitboxIDs.size(); i++)
		if (Aimbot::CompareData(d, d->traceOutputs[i], d->traceEnd[i], d->hitboxIDs[i], d->fovList[i]))
			ret = d->entID;

	return ret;
}

static int PrepareHitboxList(AimbotLoopData* d, size_t id)
{
	d->fovs[id] = 1000.f;

	d->entID = id;

	d->traceEnd.clear();
	d->hitboxIDs.clear();
	d->fovList.clear();
	d->traceOutputs.clear();

#ifdef AIMBOT_SIMD_TRACE_DATA
	d->traceEndSOA.clear();
	d->hitboxIDsSOA.clear();
	d->fovListSOA.clear();
	d->traceOutputsSOA.clear();
#endif

	HitboxList& hitboxes = d->players->hitboxes[id];

	for (size_t i = 0; i < MAX_HITBOXES; i++) {

		if (!d->hitboxList[i])
			continue;

		float fov = 0.f;

		vec3_t average = (hitboxes.start[i] + hitboxes.end[i]) * 0.5f;
		average = hitboxes.wm[i].Vector3Transform(average);

		if (!Aimbot::PreCompareData(&d->target, d->localPlayer, average, i, &fov))
			continue;

		if (d->hitboxList[i] & HitboxScanMode_t::SCAN_MULTIPOINT) {
			mvec3 mpVec = d->players->hitboxes[id].mpOffset[i] + d->players->hitboxes[id].mpDir[i] * d->players->hitboxes[id].radius[i] * pointScaleVal[i];
			mpVec = d->players->hitboxes[id].wm[i].VecSoaTransform(mpVec);

#ifdef AIMBOT_SIMD_TRACE_DATA
			d->traceEndSOA.push_back(mpVec);
			d->hitboxIDsSOA.push_back(i);
			//TODO: Convert this to more vectorizable way
			for (int o = 0; o < MULTIPOINT_COUNT; o++) {
				vec3_t angle = ((vec3_t)mpVec.acc[o] - d->localPlayer->eyePos).GetAngles(true);
				vec3_t angleDiff = (Aimbot::shootAngles - angle).NormalizeAngles<2>(-180.f, 180.f);
				float fovSOA = angleDiff.Length<2>();
				d->fovListSOA.push_back(fovSOA);
			}
			d->traceOutputsSOA.resize(d->traceOutputsSOA.size() + MULTIPOINT_COUNT);
#else
			auto rvec = mpVec.Rotate();

			//TODO: Convert this to more vectorizable way
			for (int o = 0; o < MULTIPOINT_COUNT; o++) {
				d->traceEnd.push_back(rvec.GetColAsVecp(o));
				d->hitboxIDs.push_back(i);
				vec3_t angle = ((vec3_t)rvec.GetColAsVecp(o) - d->localPlayer->eyePos).GetAngles(true);
				vec3_t angleDiff = (Aimbot::shootAngles - angle).NormalizeAngles<2>(-180.f, 180.f);
				float fov = angleDiff.Length<2>();
				d->fovList.push_back(fov);
				d->traceOutputs.push_back(0);
			}

#endif
		} else {
			d->traceEnd.push_back(average);
			d->hitboxIDs.push_back(i);
			d->fovList.push_back(fov);
			d->traceOutputs.push_back(0);
		}
	}

	return 0;
}

static int LoopPlayers(AimbotLoopData* d)
{
	int ret = 0;

	for (int i = 0; i < d->players->count; i++) {
		if (~d->ignoreList[d->players->unsortIDs[i] / 64] & (1ull << (d->players->unsortIDs[i] % 64)) && d->players->flags[i] & Flags::HITBOXES_UPDATED && ~d->players->flags[i] & Flags::FRIENDLY &&
			//The following check is just a rough way to clear the unrelated players from view. A better check would be to intersect AABB with previous target to see if they overlap. If they do not, then simply quit the loop since the players should be sorted by FOV
			d->players->fov[i] - 30.f < d->target.fov) {
			PrepareHitboxList(d, i);

			int ap = ProcessAimPoints(d);
			int aps = ProcessAimPointsSIMD(d);

			if (ap != -1) {
				d->target.id = ap;
				ret = 1;
			}

			if (aps != -1) {
				d->target.id = aps;
				ret = 1;
			}
		}
	}

#ifdef AIMBOT_THREADING
	threadSem.Post();
#endif

    return ret;
}

static void FindBestTarget(AimbotTarget* target, HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, unsigned char hitboxList[MAX_HITBOXES], uint64_t ignoreList[NumOf<64>(MAX_PLAYERS)])
{

	char backtrackMask[MAX_PLAYERS];
	float lowestFov = 1000.f;
	Players* prevPlayers = nullptr;
	Players* targetPlayers = nullptr;

	memset(backtrackMask, 0, MAX_PLAYERS);

	//First check the future, but this will be overwritten by the normal track if any of the ticks are valid
	if (futureTrack) {
		for (size_t i = 0; i < futureTrack->Count(); i += numThreads * threadQueueMultiplier) {

#ifdef AIMBOT_THREADING
			int pushedCount = 0;
#endif

			for (int o = 0; o < (int)numThreads * threadQueueMultiplier && i + o < futureTrack->Count(); o++) {
				Players& players = futureTrack->GetLastItem(i + o);
				AimbotLoopData* d = data + o;

				d->target.fov = 9000;

				//We do not want to just exit out the loop if we predicted too far into future
				if (!Tracing::BacktrackPlayers(&players, prevPlayers, backtrackMask))
					continue;

				d->target = *target;

				d->players = &players;
				d->localPlayer = localPlayer;
				d->hitboxList = hitboxList;
				d->ignoreList = ignoreList;

#ifdef AIMBOT_THREADING
				Threading::QueueJobRef(LoopPlayers, d);
				pushedCount++;
#else
				LoopPlayers(d);
#endif

				prevPlayers = &players;
			}

#ifdef AIMBOT_THREADING
			for (int i = 0; i < pushedCount; i++)
				threadSem.Wait();
#endif

			for (int o = 0; o < (int)numThreads * threadQueueMultiplier && i + o < futureTrack->Count(); o++) {
				Players& players = futureTrack->GetLastItem(i + o);
				AimbotLoopData* d = data + o;

				if (d->target.id >= 0 && d->target.fov < lowestFov) {
					lowestFov = d->target.fov;
					d->target.backTick = i + o;
					d->target.future = true;
					*target = d->target;
					targetPlayers = &players;
				}
			}
		}
	}

	bool b = false;

	for (size_t i = 0; i < track->Count() && !b; i += numThreads * threadQueueMultiplier) {
		int o = 0;

#ifdef AIMBOT_THREADING
		int pushedCount = 0;
#endif

		for (o = 0; o < (int)numThreads * threadQueueMultiplier && i + o < track->Count(); o++) {
			Players& players = track->GetLastItem(i + o);
			AimbotLoopData* d = data + o;

			d->target.fov = 9000;

			if (!Tracing::BacktrackPlayers(&players, prevPlayers, backtrackMask)) {
				b = true;
				break;
			}

			d->target = *target;

			d->players = &players;
			d->localPlayer = localPlayer;
			d->hitboxList = hitboxList;
			d->ignoreList = ignoreList;

#ifdef AIMBOT_THREADING
			Threading::QueueJobRef(LoopPlayers, d);
			pushedCount++;
#else
			LoopPlayers(d);
#endif

			prevPlayers = &players;
		}

#ifdef AIMBOT_THREADING
		for (int i = 0; i < pushedCount; i++)
			threadSem.Wait();
#endif

		for (int u = 0; u < o; u++) {
			Players& players = track->GetLastItem(i + u);
			AimbotLoopData* d = data + u;

			if (d->target.id >= 0 && (d->target.fov < lowestFov || !Tracing::VerifyTarget(targetPlayers, target->id, backtrackMask))) {
				lowestFov = d->target.fov;
				d->target.backTick = i + u;
				d->target.future = false;
				*target = d->target;
				targetPlayers = &players;
			}
		}
	}
}

AimbotTarget Aimbot::RunAimbot(HistoryList<Players, BACKTRACK_TICKS>* track, HistoryList<Players, BACKTRACK_TICKS>* futureTrack, LocalPlayer* localPlayer, unsigned char hitboxList[MAX_HITBOXES], uint64_t ignoreList[NumOf<64>(MAX_PLAYERS)], float pointScale[MAX_HITBOXES])
{
	AimbotTarget target;
	shootAngles = localPlayer->angles + localPlayer->aimOffset;
	pointScaleVal = pointScale;

	FindBestTarget(&target, track, futureTrack, localPlayer, hitboxList, ignoreList);

	if (target.id >= 0) {
		vec3_t angles = (target.targetVec - localPlayer->eyePos).GetAngles(true);
		localPlayer->angles = angles - localPlayer->aimOffset;
	}

	return target;
}
