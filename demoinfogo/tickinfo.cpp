#include "tickinfo.h"
#include <stdio.h>

TickInfo::TickInfo( int tickNumber, int roundNumber, RoundStatus roundStatus )
{
	this->tickNumber = tickNumber;
	this->roundNumber = roundNumber;
	this->roundStatus = roundStatus;
}

TickInfo::~TickInfo()
{

}

void TickInfo::AddPlayer( Player* player )
{
	Player addedPlayer = Player( player );
	playersInTick.push_back( addedPlayer );
}

void TickInfo::Print()
{
	printf( "Tick %d Info: \n", tickNumber );
	printf( "	Round number: %d\n", roundNumber );
	printf( "	Round status: %d\n", roundStatus );

	printf( "	Player Info:\n" );
	for ( std::vector< Player >::iterator it = playersInTick.begin(); it != playersInTick.end(); ++it )
	{			
		if ( ( *it ).GetIsConnected() )
		{				
			( *it ).Print();
		}
		//( *it )->Print();
	}
}