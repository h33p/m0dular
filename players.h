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
#include "utils/shared_utils.h"
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

struct Players
{
	vec3_t* boundsStart;
	vec3_t* boundsEnd;
	vec3_t* origin;
	vec3_t* eyePos;
	vec3_t* velocity;
	CapsuleColliderSOA<SIMD_COUNT> (*colliders)[NumOfSIMD(MAX_HITBOXES)];
	HitboxList* hitboxes;
	void** instance;
	int* flags;
	int* health;
	int* armor;
	float* time;
	char (*name)[NAME_LEN];
	float* fov;
	matrix<3,4> (*bones)[MAX_BONES];
	//Used for sorting the player
	int sortIDs[MAX_PLAYERS];
	int unsortIDs[MAX_PLAYERS];
	int count;
	float globalTime;

	static constexpr size_t sizePerPlayer = sizeof(boundsStart[0]) + sizeof(boundsEnd[0]) + sizeof(origin[0]) + sizeof(eyePos[0]) + sizeof(velocity[0]) + sizeof(colliders[0]) + sizeof(hitboxes[0]) + sizeof(instance[0]) + sizeof(flags[0]) + sizeof(health[0]) + sizeof(armor[0]) + sizeof(time[0]) + sizeof(name[0]) + sizeof(fov[0]) + sizeof(bones[0]);
	static constexpr size_t extraAlignmentNeeds = alignof(vec3_t) * 5 + alignof(decltype(colliders[0])) + alignof(decltype(hitboxes[0])) + alignof(void*) + alignof(int) * 3 + alignof(float) + alignof(char*) + alignof(float) + alignof(decltype(bones));

	const auto& operator=(Players& o)
	{
		memcpy(this, &o, sizeof(Players));
		return *this;
	}

	int Resort(const Players& target, int id)
	{
		int uid = unsortIDs[id];
		if (uid >= 0 && uid < MAX_PLAYERS) {
			int sid = target.sortIDs[uid];
			if (sid >= 0 && sid < MAX_PLAYERS && sid < target.count)
				return sid;
		}
		return MAX_PLAYERS;
	}

	void FreeAll()
	{
		if (count && boundsStart) {
			free((void*)boundsStart);
		}

		memset(this, 0, sizeof(*this));
		memset(sortIDs, -1, sizeof(sortIDs));
		memset(unsortIDs, -1, sizeof(sortIDs));
	}

	void Allocate(int cnt)
	{
		FreeAll();
		count = cnt;

		void* data = malloc(sizePerPlayer * count + extraAlignmentNeeds);

		//We have to align some of the data
		boundsStart = (vec3_t*)data;
		boundsEnd = AlignUp(boundsStart + count);
		origin = AlignUp(boundsEnd + count);
		eyePos = AlignUp(origin + count);
		velocity = AlignUp(eyePos + count);
		colliders = AlignUp((decltype(colliders))(velocity + count));
		hitboxes = AlignUp((decltype(hitboxes))(colliders + count));
		instance = AlignUp((decltype(instance))(hitboxes + count));
		flags = AlignUp((decltype(flags))(instance + count));
		health = AlignUp(flags + count);
		armor = AlignUp(health + count);
		time = AlignUp((decltype(time))(armor + count));
		name = AlignUp((decltype(name))(time + count));
		fov = AlignUp((decltype(fov))(name + count));
		bones = AlignUp((decltype(bones))(fov + count));

		memset(sortIDs, -1, sizeof(sortIDs));
		memset(unsortIDs, -1, sizeof(sortIDs));
	}

	Players(int cnt)
	{
	    Allocate(cnt);
	}

	Players()
	{
		memset(this, 0, sizeof(*this));
		memset(sortIDs, -1, sizeof(sortIDs));
		memset(unsortIDs, -1, sizeof(sortIDs));
	}

	~Players()
	{
		FreeAll();
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
