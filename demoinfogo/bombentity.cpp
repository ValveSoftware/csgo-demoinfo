#include "bombentity.h"

BombEntity::BombEntity()
{
	this->x = -1;
	this->y = -1;
	this->z = -1;
	this->bombStatus = BOMB_ON_PLAYER;
	this->bombTimer = BOMB_TIMER_TICKS;
}

BombEntity::BombEntity( BombEntity* bomb )
{
	this->x = bomb->x;
	this->y = bomb->y;
	this->z = bomb->z;
	this->bombStatus = bomb->bombStatus;
	this->bombTimer = bomb->bombTimer;
}

BombEntity::~BombEntity()
{

}

void BombEntity::TickCleanUp( int tickDifference )
{
	if ( bombStatus == BOMB_PLANTED )
	{
		if ( bombTimer > 0 )
		{
			bombTimer -= tickDifference;
		}
		else
		{
			bombStatus = BOMB_PRE_EXPLOSION;
		}
	}
}

void BombEntity::RoundCleanUp()
{
	this->x = -1;
	this->y = -1;
	this->z = -1;
	this->bombStatus = BOMB_ON_PLAYER;
	this->bombTimer = BOMB_TIMER_TICKS;
}

void BombEntity::Print()
{	
	//We only care if it's on the map
	if ( bombStatus != BOMB_ON_PLAYER )
	{
		printf( "	Bomb Info:\n" );
		printf( "	----- Location: %f, %f, %f\n", x, y, z );
	}
	else
	{
		return;
	}

	switch ( bombStatus )
	{
		case BOMB_PLANTED:		
			{
				printf( "	----- Status: Planted: %d ticks left.\n", bombTimer );
			}
			break;

		case BOMB_EXPLODED:		
			{
				printf( "	----- Status: Exploded.\n" );
			}
			break;

		case BOMB_PRE_EXPLOSION:		
			{
				printf( "	----- Status: About to explode, can't defuse.\n" );		
			}
			break;

		case BOMB_DEFUSED:		
			{
				printf( "	----- Status: Defused.\n" );	
			}
			break;

		case BOMB_DROPPED:		
			{
				printf( "	----- Status: Dropped.\n" );
			}
			break;

		default:
			break;
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

int BombEntity::GetBombTimer()
{
	return this->bombTimer;
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

void BombEntity::SetBombStatus( BombStatus bombStatus )
{
	this->bombStatus = bombStatus;
}