#ifndef PLAYERDEATHEVENT_H
#define PLAYERDEATHEVENT_H

#include "enums.h"
#include <stdio.h>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class PlayerDeathEvent
{
public:
	PlayerDeathEvent();
	PlayerDeathEvent( int playerGUID, int attackerGUID, int assisterGUID, bool assistedFlash, std::string weaponName, bool headshot, bool wallbang, bool throughSmoke, bool whileBlind, float distance );
	PlayerDeathEvent( PlayerDeathEvent* playerDeath );
	~PlayerDeathEvent();

	void Print();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE( PlayerDeathEvent, playerGUID, attackerGUID, assisterGUID, assistedFlash, weaponName, headshot, wallbang, throughSmoke, whileBlind, distance );

private:
	int playerGUID;
	int attackerGUID;
	int assisterGUID;
	bool assistedFlash;
	std::string weaponName;
	bool headshot;
	bool wallbang;
	bool throughSmoke;
	bool whileBlind;
	float distance;

};

#endif