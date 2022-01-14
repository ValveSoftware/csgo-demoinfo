#include "playerhurtevent.h"

PlayerHurtEvent::PlayerHurtEvent()
{

}

PlayerHurtEvent::PlayerHurtEvent( int playerGUID, int attackerGUID, int healthDamage, int armourDamage, std::string weaponName, int hitgroup )
{
	this->playerGUID = playerGUID;
	this->attackerGUID = attackerGUID;
	this->healthDamage = healthDamage;
	this->armourDamage = armourDamage;
	this->weaponName = weaponName;
	this->hitgroup = hitgroup;
}

PlayerHurtEvent::PlayerHurtEvent( PlayerHurtEvent* playerHurt )
{
	this->playerGUID = playerHurt->playerGUID;
	this->attackerGUID = playerHurt->attackerGUID;
	this->healthDamage = playerHurt->healthDamage;
	this->armourDamage = playerHurt->armourDamage;
	this->weaponName = playerHurt->weaponName;
	this->hitgroup = playerHurt->hitgroup;
}

PlayerHurtEvent::~PlayerHurtEvent()
{
}

void PlayerHurtEvent::Print()
{
	if ( attackerGUID != -404 )
	{
		printf( "	----- Player %d hurt for %d damage (%d armour) by %d (weapon: %s, hitgroup: %d).\n", playerGUID, healthDamage, armourDamage, attackerGUID, weaponName.c_str(), hitgroup );
	}
	else
	{
		printf( "	----- Player %d hurt for %d damage (%d armour) by world.\n", playerGUID, healthDamage, armourDamage );
	}
}