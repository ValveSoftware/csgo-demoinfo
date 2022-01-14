#ifndef PLAYERHURTEVENT_H
#define PLAYERHURTEVENT_H

#include "enums.h"
#include <stdio.h>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class PlayerHurtEvent
{
public:
	PlayerHurtEvent();
	PlayerHurtEvent( int playerGUID, int attackerGUID, int healthDamage, int armourDamage, std::string weaponName, int hitgroup );
	PlayerHurtEvent( PlayerHurtEvent* playerHurt );
	~PlayerHurtEvent();

	void Print();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE( PlayerHurtEvent, playerGUID, attackerGUID, healthDamage, armourDamage, weaponName, hitgroup );

private:	
	int playerGUID;
	int attackerGUID;
	int healthDamage;
	int armourDamage;
	std::string weaponName;
	int hitgroup;
};

#endif