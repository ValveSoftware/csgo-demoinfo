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

	void RoundCleanUp();
	void Print();
	
	float GetX();
	float GetY();
	float GetZ();
	bool GetIsPlanted();
	bool GetIsOnPlayer();
	bool GetIsDetonated();
	bool GetIsDefused();
	void SetX( float x );
	void SetY( float y );
	void SetZ( float z );
	void SetIsPlanted( bool isPlanted );
	void SetIsOnPlayer( bool isOnPlayer );
	void SetIsDetonated( bool isDetonated );
	void SetIsDefused( bool isDefused );

private:	
	float x;
	float y;
	float z;
	bool isPlanted;
	bool isOnPlayer;
	bool isDetonated;
	bool isDefused;
};

#endif