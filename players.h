#ifndef PLAYERS_H
#define PLAYERS_H

/*
 * Here is the basic data for the players.
 * It is used by the various features of the framework.
 * Game specific functions are needed to be implemented
 * to fill in the data.
*/

#include "math/mmath.h"
#include <string.h>

constexpr int PLAYER_CHUNKS = NumOfSIMD(MAX_PLAYERS);
const int NAME_LEN = 32;
const int MAX_HITBOXES = 16;
constexpr int HITBOX_CHUNKS = NumOfSIMD(MAX_HITBOXES);

enum Flags
{
	EXISTS = (1 << 0),
	UPDATED = (1 << 1),
	ONGROUND = (1 << 2),
	DUCKING = (1 << 3)
};

struct  __attribute__((aligned(SIMD_COUNT * 4)))
HitboxList
{
	nvec3 start[HITBOX_CHUNKS];
	nvec3 end[HITBOX_CHUNKS];
	matrix<3,4> wm[MAX_HITBOXES];
	//Radius, damage multiplier
	nvec<2> data[HITBOX_CHUNKS];
};

struct __attribute__((aligned(SIMD_COUNT * 4)))
Players
{
	nvec3 origin[PLAYER_CHUNKS];
	nvec3 boundsStart[PLAYER_CHUNKS];
	nvec3 boundsEnd[PLAYER_CHUNKS];
	HitboxList hitboxes[MAX_PLAYERS];
	vec3_t velocity[MAX_PLAYERS];
	void* instance[MAX_PLAYERS];
	int flags[MAX_PLAYERS];
	int health[MAX_PLAYERS];
	int armor[MAX_PLAYERS];
	float time[MAX_PLAYERS];
	char name[MAX_PLAYERS][NAME_LEN];
	//Used for sorting the player
	int sortIDs[MAX_PLAYERS];
	float fov[MAX_PLAYERS];
	int count;

	const auto& operator=(Players& o)
	{
		memcpy(this, &o, sizeof(Players));
		return *this;
	}
};

struct __attribute__((aligned(SIMD_COUNT * 4)))
LocalPlayer
{
	vec3_t eyePos;
	vec3_t angles;
	float time;
	int weaponAmmo;
	float weaponDamage;
	float weaponPenetration;
	float weaponArmorPenetration;
	float weaponRange;
};

#endif
