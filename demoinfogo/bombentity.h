#ifndef BOMBENTITY_H
#define BOMBENTITY_H

#include "enums.h"
#include <stdio.h>

class BombEntity
{
public:
	BombEntity();
	BombEntity( BombEntity* bomb );
	~BombEntity();

	void TickCleanUp( int tickDifference );
	void RoundCleanUp();
	void Print();
	
	float GetX();
	float GetY();
	float GetZ();
	int GetBombTimer();
	BombStatus GetBombStatus();
	void SetX( float x );
	void SetY( float y );
	void SetZ( float z );
	void SetBombStatus( BombStatus bombStatus );

private:	
	float x;
	float y;
	float z;
	BombStatus bombStatus;
	int bombTimer;
};

#endif