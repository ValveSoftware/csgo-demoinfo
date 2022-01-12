#ifndef TICKINFO_H
#define TICKINFO_H

#include "enums.h"
#include "player.h"
#include "bombentity.h"
#include "grenadeentity.h"
#include "playerhurtevent.h"
#include "playerdeathevent.h"
#include <vector>

class TickInfo
{
public:
	TickInfo( int tickNumber, int roundNumber, RoundStatus roundStatus );
	~TickInfo();

	void AddPlayer( Player* player );
	void AddBomb( BombEntity* bomb );
	void AddGrenade( GrenadeEntity* grenade );
	void AddPlayerHurt( PlayerHurtEvent* playerHurt );
	void AddPlayerDeath( PlayerDeathEvent* playerDeath );
	void Print();

private:
	int tickNumber;
	int roundNumber;
	RoundStatus roundStatus;
	std::vector< Player > playersInTick;
	BombEntity bomb;
	std::vector< GrenadeEntity > grenadesInTick;
	std::vector< PlayerHurtEvent > hurtEventsInTick;
	std::vector< PlayerDeathEvent > deathEventsInTick;
};

#endif
