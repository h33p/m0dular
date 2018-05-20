#ifndef PLAYERS_H
#define PLAYERS_H

#include "math/mmath.h"

const int MAX_PLAYERS = 128;
constexpr int PLAYER_CHUNKS = NumOfSIMD(MAX_PLAYERS);
const int NAME_LEN = 32;

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
