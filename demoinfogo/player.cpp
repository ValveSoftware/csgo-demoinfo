#include "player.h"
#include <stdio.h>

Player::Player()
{
	
}

Player::Player( int GUID, int entityID, int userID, std::string name, bool isBot )
{
	this->GUID = GUID;
	this->isConnected = true;
	this->isBot = isBot;
	this->entityID = entityID;
	this->userID = userID;
	this->name = name;
	this->teamID = -1;
	this->x = -1;
	this->y = -1;
	this->z = -1;
	this->eyePitch = -1;
	this->eyeYaw = -1;
	this->health = -1;
	this->armour = -1;
	this->hasHelmet = false;
	this->hasDefuseKit = false;
	this->money = -1;
	this->flashDuration = -1;
	this->status = PLAYER_DEFAULT;
	this->hasBomb = false;
}

Player::Player( Player* player )
{
	this->GUID = player->GUID;
	this->isConnected = player->isConnected;
	this->isBot = player->isBot;
	this->entityID = player->entityID;
	this->userID = player->userID;
	this->name = player->name;
	this->teamID = player->teamID;
	this->x = player->x;
	this->y = player->y;
	this->z = player->z;
	this->eyePitch = player->eyePitch;
	this->eyeYaw = player->eyeYaw;
	this->health = player->health;
	this->armour = player->armour;
	this->hasHelmet = player->hasHelmet;
	this->hasDefuseKit = player->hasDefuseKit;
	this->money = player->money;
	this->flashDuration = player->flashDuration;
	this->status = player->status;
	this->hasBomb = player->hasBomb;
}

Player::~Player()
{

}

bool Player::CheckBlind()
{
	return flashDuration > 0;
}

bool Player::CheckDead()
{
	return health <= 0;
}

void Player::TickCleanUp()
{
	//If a player was planting/defusing on the previous tick, they'll keep planting/defusing
	//Firing a weapon/dying/aborting will trump their planting/defusing state automatically
	if ( status != PLAYER_PLANTING && status != PLAYER_DEFUSING )
	{
		status = PLAYER_DEFAULT;
	}
}

void Player::RoundCleanUp()
{
	hasBomb = false;
	status = PLAYER_DEFAULT;
}

void Player::Print()
{
	const char* helmetString = ( hasHelmet ) ? " (Helmet)" : "";
	const char* defuseKitString = ( hasDefuseKit ) ? ", Kit" : "";
	printf( "	----- Player %s (GUID: %d, Entity: %d, UserID: %d, Team: %d) -----\n", name.c_str(), GUID, entityID, userID, teamID );
	printf( "		----- Position: %.2f, %.2f, %.2f \n", x, y, z );
	printf( "		----- Looking: %.2f, %.2f \n", eyePitch, eyeYaw );
	printf( "		----- Health: %d, Armour%s: %d%s \n", health, helmetString, armour, defuseKitString );
	printf( "		----- Money: %d \n", money );
	printf( "		----- Status: %d \n", status );

	if ( CheckBlind() )
	{
		printf( "		----- Is flashed \n" );
	}
	if ( hasBomb )
	{
		printf( "		----- Holding bomb \n" );		
	}
}

int Player::GetGUID()
{
	return GUID;
}

bool Player::GetIsConnected()
{
	return this->isConnected;
}

bool Player::GetIsBot()
{
	return this->isBot;
}

int Player::GetUserID()
{
	return this->userID;
}

std::string Player::GetName()
{
	return this->name;
}

bool Player::GetHasBomb()
{
	return this->hasBomb;
}

void Player::SetIsConnected( bool isConnected )
{
	this->isConnected = isConnected;
}

void Player::SetStatus( PlayerStatus status )
{
	this->status = status;
}

void Player::SetHasBomb( bool hasBomb )
{
	this->hasBomb = hasBomb;
}