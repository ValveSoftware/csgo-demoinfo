#include "bombentity.h"

BombEntity::BombEntity()
{
	this->x = -1;
	this->y = -1;
	this->z = -1;
	this->isPlanted = false;
	this->isOnPlayer = true;
	this->isDetonated = false;
	this->isDefused = false;
}

BombEntity::BombEntity( BombEntity* bomb )
{
	this->x = bomb->x;
	this->y = bomb->y;
	this->z = bomb->z;
	this->isPlanted = bomb->isPlanted;
	this->isOnPlayer = bomb->isOnPlayer;
	this->isDetonated = bomb->isDetonated;
	this->isDefused = bomb->isDefused;
}

BombEntity::~BombEntity()
{

}

void BombEntity::RoundCleanUp()
{
	this->x = -1;
	this->y = -1;
	this->z = -1;
	this->isPlanted = false;
	this->isOnPlayer = true;
	this->isDetonated = false;
	this->isDefused = false;
}

void BombEntity::Print()
{
	//We only care if it's on the map
	if ( !isOnPlayer )
	{
		printf( "	----- Bomb Info:\n" );
		printf( "		----- Location: %f, %f, %f\n", x, y, z );
		if ( isPlanted )
		{
			if ( isDetonated )
			{
				printf( "		----- Status: Exploded.\n" );
			}
			else if ( isDefused )
			{
				printf( "		----- Status: Defused.\n" );
			}
			else{
				printf( "		----- Status: Planted.\n" );
			}
		}
		else
		{
			printf( "		----- Status: Dropped.\n" );
		}
	}
}

float BombEntity::GetX()
{
	return this->x;
}

float BombEntity::GetY()
{
	return this->y;
}

float BombEntity::GetZ()
{
	return this->z;
}

bool BombEntity::GetIsPlanted()
{
	return this->isPlanted;
}

bool BombEntity::GetIsOnPlayer()
{
	return this->isOnPlayer;
}

bool BombEntity::GetIsDetonated()
{
	return this->isDetonated;
}

bool BombEntity::GetIsDefused()
{
	return this->isDefused;
}

void BombEntity::SetX( float x )
{
	this->x = x;
}

void BombEntity::SetY( float y )
{
	this->y = y;
}

void BombEntity::SetZ( float z )
{
	this->z = z;
}

void BombEntity::SetIsPlanted( bool isPlanted )
{
	this->isPlanted = isPlanted;
}

void BombEntity::SetIsOnPlayer( bool isOnPlayer )
{
	this->isOnPlayer = isOnPlayer;
}

void BombEntity::SetIsDetonated( bool isDetonated )
{
	this->isDetonated = isDetonated;
}

void BombEntity::SetIsDefused( bool isDefused )
{
	this->isDefused = isDefused;
}