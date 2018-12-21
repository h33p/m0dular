#ifndef AIMBOT_TYPES_H
#define AIMBOT_TYPES_H

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

#endif
