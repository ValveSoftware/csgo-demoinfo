#ifndef PLAYERHURTEVENT_H
#define PLAYERHURTEVENT_H

#include "enums.h"
#include <stdio.h>
#include <string>

class PlayerHurtEvent
{
public:
	PlayerHurtEvent( int playerGUID, int attackerGUID, int healthDamage, int armourDamage, std::string weaponName, int hitgroup );
	PlayerHurtEvent( PlayerHurtEvent* playerHurt );
	~PlayerHurtEvent();

	void Print();

private:	
	int playerGUID;
	int attackerGUID;
	int healthDamage;
	int armourDamage;
	std::string weaponName;
	int hitgroup;
};

#endif