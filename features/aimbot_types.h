#ifndef AIMBOT_TYPES_H
#define AIMBOT_TYPES_H

struct LocalPlayer;
struct Players;

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

struct AimbotLoopData {
	AimbotTarget target;
	LocalPlayer* localPlayer;
	Players* players;

	unsigned char* hitboxList;
	uint64_t* ignoreList;

	float fovs[MAX_PLAYERS];

	int entID;

	std::vector<vec3_t> traceEnd;
	std::vector<int> hitboxIDs;
	std::vector<float> fovList;
	std::vector<int> traceOutputs;

#ifdef AIMBOT_SIMD_TRACE_DATA
	std::vector<mvec3> traceEndSOA;
	std::vector<int> hitboxIDsSOA;
	std::vector<float> fovListSOA;
	std::vector<int> traceOutputsSOA;
#endif
};

#endif
