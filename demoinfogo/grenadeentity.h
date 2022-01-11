#ifndef GRENADEENTITY_H
#define GRENADEENTITY_H

#include "enums.h"
#include <stdio.h>

class GrenadeEntity
{
public:
	GrenadeEntity( GrenadeType grenadeType, float x, float y, float z, int entityID );
	GrenadeEntity( GrenadeEntity* grenade );
	~GrenadeEntity();

	bool TickCleanUp( int tickDifference );
	void Print();
	
	float GetX();
	float GetY();
	float GetZ();
	int GetEntityID();
	int GetGrenadeTimer();
	bool GetReadyToRemove();
	void SetReadyToRemove( bool readyToRemove );
	void SetDecoyFiring( bool decoyFiring );

private:	
	GrenadeType grenadeType;
	float x;
	float y;
	float z;
	int entityID;
	int grenadeTimer;
	bool readyToRemove;
	bool decoyFiring;
};

#endif