#include "playerdeathevent.h"

PlayerDeathEvent::PlayerDeathEvent()
{
	
}

PlayerDeathEvent::PlayerDeathEvent( int playerGUID, int attackerGUID, int assisterGUID, bool assistedFlash, std::string weaponName, bool headshot, bool wallbang, bool throughSmoke, bool whileBlind, float distance )
{
	this->playerGUID = playerGUID;
	this->attackerGUID = attackerGUID;
	this->assisterGUID = assisterGUID;
	this->assistedFlash = assistedFlash;
	this->weaponName = weaponName;
	this->headshot = headshot;
	this->wallbang = wallbang;
	this->throughSmoke = throughSmoke;
	this->whileBlind = whileBlind;
	this->distance = distance;
}

PlayerDeathEvent::PlayerDeathEvent( PlayerDeathEvent* playerDeath )
{
	this->playerGUID = playerDeath->playerGUID;
	this->attackerGUID = playerDeath->attackerGUID;
	this->assisterGUID = playerDeath->assisterGUID;
	this->assistedFlash = playerDeath->assistedFlash;
	this->weaponName = playerDeath->weaponName;
	this->headshot = playerDeath->headshot;
	this->wallbang = playerDeath->wallbang;
	this->throughSmoke = playerDeath->throughSmoke;
	this->whileBlind = playerDeath->whileBlind;
	this->distance = playerDeath->distance;
}

PlayerDeathEvent::~PlayerDeathEvent()
{
}

void PlayerDeathEvent::Print()
{
	if ( attackerGUID != -404 )
	{
		if ( assisterGUID != -404 )
		{
			printf( "	----- Player %d died to %d (Assist: %d). -----\n", playerGUID, attackerGUID, assisterGUID );
		}
		else
		{
			printf( "	----- Player %d died to %d. -----\n", playerGUID, attackerGUID );			
		}
	}
	else
	{
		printf( "	----- Player %d died. -----\n", playerGUID );
	}
	printf( "		----- Weapon: %s \n", weaponName.c_str() );
	if ( assistedFlash )
	{		
		printf( "		----- Flash assist \n" );
	}
	if ( headshot )
	{
		printf( "		----- Headshot \n");
	}
	if ( wallbang )
	{
		printf( "		----- Wallbang \n");
	}
	if ( throughSmoke )
	{
		printf( "		----- Through smoke \n");
	}
	if ( whileBlind )
	{
		printf( "		------ While blind \n");
	}
}