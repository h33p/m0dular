#ifndef PLAYERS_H
#define PLAYERS_H

/*
 * Here is the basic data for the players.
 * It is used by the various features of the framework.
 * Game specific functions are needed to be implemented
 * to fill in the data.
*/

#include "math/mmath.h"

constexpr int PLAYER_CHUNKS = NumOfSIMD(MAX_PLAYERS);
const int NAME_LEN = 32;
const int MAX_HITBOXES = 32;
constexpr int HITBOX_CHUNKS = NumOfSIMD(MAX_HITBOXES);

struct HitboxList
{
	nvec3 start[HITBOX_CHUNKS];
	nvec3 end[HITBOX_CHUNKS];
	matrix4x4 w2s[MAX_HITBOXES];
};

struct Players
{
	nvec3 origin[PLAYER_CHUNKS];
	nvec3 boundsStart[PLAYER_CHUNKS];
	nvec3 boundsEnd[PLAYER_CHUNKS];
	vec3_t velocity[MAX_PLAYERS];
	void** instance[MAX_PLAYERS];
	float** hitbox[MAX_PLAYERS];
	int health[MAX_PLAYERS];
	int armor[MAX_PLAYERS];
	char name[MAX_PLAYERS][NAME_LEN];
};

#endif
