#ifndef BOMBENTITY_H
#define BOMBENTITY_H

#include "enums.h"
#include <stdio.h>

#include "json.hpp"
using json = nlohmann::json;

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

	NLOHMANN_DEFINE_TYPE_INTRUSIVE( BombEntity, x, y, z, bombStatus, bombTimer );

private:	
	float x;
	float y;
	float z;
	BombStatus bombStatus;
	int bombTimer;
};

#endif