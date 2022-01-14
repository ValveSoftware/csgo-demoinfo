#include "grenadeentity.h"

GrenadeEntity::GrenadeEntity()
{
	
}

//TODO can add thrower and thrower origin maybe
GrenadeEntity::GrenadeEntity( GrenadeType grenadeType, float x, float y, float z, int entityID )
{
	this->grenadeType = grenadeType;
	this->x = x;
	this->y = y;
	this->z = z;
	this->entityID = entityID;
	this->readyToRemove = false;
	this->decoyFiring = false;
	switch ( grenadeType )
	{		
		case G_MOLOTOV:		
			{
				this->grenadeTimer = MOLOTOV_TIMER_TICKS;
			}
			break;

		case G_SMOKE:		
			{
				this->grenadeTimer = SMOKE_TIMER_TICKS;
			}
			break;

		case G_DECOY:		
			{
				this->grenadeTimer = DECOY_TIMER_TICKS;
			}
			break;

		default:
			{
				//Kinda jank but can change this so nades "last" multiple ticks
				this->grenadeTimer = 0;
			}
	}
}

GrenadeEntity::GrenadeEntity( GrenadeEntity* grenade )
{
	this->grenadeType = grenade->grenadeType;
	this->x = grenade->x;
	this->y = grenade->y;
	this->z = grenade->z;
	this->entityID = grenade->entityID;
	this->grenadeTimer = grenade->grenadeTimer;
	this->readyToRemove = grenade->readyToRemove;
	this->decoyFiring = grenade->decoyFiring;
}

GrenadeEntity::~GrenadeEntity()
{

}

bool GrenadeEntity::TickCleanUp( int tickDifference )
{
	if ( grenadeTimer - tickDifference >= 0)
	{
		grenadeTimer -= tickDifference;
		return false;
	}
	else if ( grenadeType == G_FLASH || grenadeType == G_HE )
	{
		readyToRemove = true;
		return true;
	}
	decoyFiring = false;
	return false;
}

void GrenadeEntity::Print()
{	
	const char* grenadeString;

	switch ( grenadeType )
	{		
		case G_MOLOTOV:		
			{
				grenadeString = "Molotov";
			}
			break;

		case G_SMOKE:		
			{
				grenadeString = "Smoke";
			}
			break;

		case G_FLASH:		
			{
				grenadeString = "Flashbang";
			}
			break;

		case G_HE:		
			{
				grenadeString = "HE";
			}
			break;

		case G_DECOY:		
			{
				grenadeString = "Decoy";
			}
			break;

		default:
			break;
	}
	printf( "	----- %s (EID: %d) at %f, %f, %f. Time left: %d\n", grenadeString, entityID, x, y, z, grenadeTimer );

	if ( decoyFiring )
	{		
		printf( "		----- Decoy fired\n" );
	}	
}

float GrenadeEntity::GetX()
{
	return this->x;
}

float GrenadeEntity::GetY()
{
	return this->y;
}

float GrenadeEntity::GetZ()
{
	return this->z;
}

int GrenadeEntity::GetEntityID()
{
	return this->entityID;
}

int GrenadeEntity::GetGrenadeTimer()
{
	return this->grenadeTimer;
}

bool GrenadeEntity::GetReadyToRemove()
{
	return this->readyToRemove;
}

void GrenadeEntity::SetReadyToRemove( bool readyToRemove )
{
	this->readyToRemove = readyToRemove;
}

void GrenadeEntity::SetDecoyFiring( bool decoyFiring )
{
	this->decoyFiring = decoyFiring;
}