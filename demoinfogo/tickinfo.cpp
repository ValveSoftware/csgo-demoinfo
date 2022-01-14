#include "tickinfo.h"
#include <stdio.h>

TickInfo::TickInfo(){}

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

void TickInfo::AddBomb( BombEntity* bomb )
{
	this->bomb = BombEntity( bomb );
}

void TickInfo::AddGrenade( GrenadeEntity* grenade )
{
	GrenadeEntity addedGrenade = GrenadeEntity( grenade );
	grenadesInTick.push_back( addedGrenade );
}

void TickInfo::AddPlayerHurt( PlayerHurtEvent* playerHurt )
{
	PlayerHurtEvent addedPlayerHurt = PlayerHurtEvent( playerHurt );
	hurtEventsInTick.push_back( addedPlayerHurt );
}

void TickInfo::AddPlayerDeath( PlayerDeathEvent* playerDeath )
{
	PlayerDeathEvent addedPlayerDeath = PlayerDeathEvent( playerDeath );
	deathEventsInTick.push_back( addedPlayerDeath );
}

void TickInfo::Print()
{
	printf( "\nTick %d Info: \n", tickNumber );
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

	bomb.Print();

	if ( grenadesInTick.size() > 0 )
	{		
		printf( "	Grenade Info:\n" );
		for ( std::vector< GrenadeEntity >::iterator it = grenadesInTick.begin(); it != grenadesInTick.end(); ++it )
		{			
			( *it ).Print();
		}
	}

	if ( hurtEventsInTick.size() > 0 )
	{
		printf( "	Hurt Info:\n" );
		for ( std::vector< PlayerHurtEvent >::iterator it = hurtEventsInTick.begin(); it != hurtEventsInTick.end(); ++it )
		{
			( *it ).Print();
		}
	}

	if ( deathEventsInTick.size() > 0 )
	{
		printf( "	Death Info:\n" );
		for ( std::vector< PlayerDeathEvent >::iterator it = deathEventsInTick.begin(); it != deathEventsInTick.end(); ++it )
		{
			( *it ).Print();
		}
	}
}

size_t TickInfo::SizeTest()
{
	size_t sum = 0;

	for ( std::vector< Player >::iterator it = playersInTick.begin(); it != playersInTick.end(); ++it )
	{			
		sum += sizeof(*it);
	}

	for ( std::vector< GrenadeEntity >::iterator it = grenadesInTick.begin(); it != grenadesInTick.end(); ++it )
	{			
		sum += sizeof(*it);
	}

	for ( std::vector< PlayerHurtEvent >::iterator it = hurtEventsInTick.begin(); it != hurtEventsInTick.end(); ++it )
	{			
		sum += sizeof(*it);
	}

	for ( std::vector< PlayerDeathEvent >::iterator it = deathEventsInTick.begin(); it != deathEventsInTick.end(); ++it )
	{			
		sum += sizeof(*it);
	}

	return sum;
}