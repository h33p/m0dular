#ifndef PLAYERS_H
#define PLAYERS_H

/*
 * Here is the basic data for the players.
 * It is used by the various features of the framework.
 * Game specific functions are needed to be implemented
 * to fill in the data.
*/

#include "math/mmath.h"
#include "utils/intersect.h"
#include <string.h>

constexpr int PLAYER_CHUNKS = NumOfSIMD(MAX_PLAYERS);
const int NAME_LEN = 32;
const int MAX_HITBOXES = 16;
constexpr int HITBOX_CHUNKS = NumOfSIMD(MAX_HITBOXES);

#ifndef MULTIPOINT_COUNT
constexpr size_t MULTIPOINT_COUNT = 8;
#endif
using mvec3 = vec3soa<float, MULTIPOINT_COUNT>;

/*
  UPDATED is set when EXISTS is set and something was updated
  HITBOXES_UPDATED is set when all the hitbox data was updated (so that aimbot data is correct)
  Other flags are self-explanatory
*/

enum Flags
{
	EXISTS = (1 << 0),
	UPDATED = (1 << 1),
	ONGROUND = (1 << 2),
	DUCKING = (1 << 3),
	HITBOXES_UPDATED = (1 << 4),
	FRIENDLY = (1 << 5)
};

enum Keys
{
	ATTACK1 = (1 << 0),
	ATTACK2 = (1 << 1),
	JUMP = (1 << 2)
};

/*
  Somewhat localized, since this is lare enough data which would make the
	reads jump around a lot when accessing very similar data.
*/
struct alignas(SIMD_COUNT * 4)
HitboxList
{
	matrix<3,4> wm[MAX_HITBOXES];

	vec3_t start[MAX_HITBOXES];
	vec3_t end[MAX_HITBOXES];

	float damageMul[MAX_HITBOXES];
	float radius[MAX_HITBOXES];

	mvec3 mpOffset[MAX_HITBOXES];
	mvec3 mpDir[MAX_HITBOXES];
};

/*
  All player data is sorted in some fashion.
  To access the player by its internal ID, use the sortIDs member
*/

struct alignas(SIMD_COUNT * 4)
Players
{
	nvec3 boundsStart[PLAYER_CHUNKS];
	nvec3 boundsEnd[PLAYER_CHUNKS];
	vec3_t origin[MAX_PLAYERS];
	vec3_t velocity[MAX_PLAYERS];
	CapsuleColliderSOA<SIMD_COUNT> colliders[MAX_PLAYERS][NumOfSIMD(MAX_HITBOXES)];
	HitboxList hitboxes[MAX_PLAYERS];
	void* instance[MAX_PLAYERS];
	int flags[MAX_PLAYERS];
	int health[MAX_PLAYERS];
	int armor[MAX_PLAYERS];
	float time[MAX_PLAYERS];
	char name[MAX_PLAYERS][NAME_LEN];
	//Used for sorting the player
	int sortIDs[MAX_PLAYERS];
	int unsortIDs[MAX_PLAYERS];
	float fov[MAX_PLAYERS];
	int count;

	//Late in the list, since this data is a couple of pages long
	matrix<3,4> bones[MAX_PLAYERS][MAX_BONES];

	const auto& operator=(Players& o)
	{
		memcpy(this, &o, sizeof(Players));
		return *this;
	}
};

struct alignas(SIMD_COUNT * 4)
LocalPlayer
{
	vec3_t eyePos;
	vec3_t angles;
	vec3_t aimOffset;
	vec3_t origin;
	vec3_t velocity;
	float time;
	int weaponAmmo;
	float weaponDamage;
	float weaponPenetration;
	float weaponArmorPenetration;
	float weaponRange;
	float weaponRangeModifier;
	int keys;
	int flags;
	int ID;
};

#endif
