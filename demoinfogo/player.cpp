#include "player.h"
#include <stdio.h>

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

void Player::SetIsConnected( bool isConnected )
{
	this->isConnected = isConnected;
}

void Player::SetStatus( PlayerStatus status )
{
	this->status = status;
}