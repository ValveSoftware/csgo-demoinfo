#ifndef TICKINFO_H
#define TICKINFO_H

#include "enums.h"
#include "player.h"
#include "bombentity.h"
#include "grenadeentity.h"
#include "playerhurtevent.h"
#include "playerdeathevent.h"
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

class TickInfo
{
public:
	TickInfo();
	TickInfo( int tickNumber, int roundNumber, RoundStatus roundStatus );
	~TickInfo();

	void AddPlayer( Player* player );
	void AddBomb( BombEntity* bomb );
	void AddGrenade( GrenadeEntity* grenade );
	void AddPlayerHurt( PlayerHurtEvent* playerHurt );
	void AddPlayerDeath( PlayerDeathEvent* playerDeath );
	void Print();
	size_t SizeTest();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE( TickInfo, tickNumber, roundNumber, roundStatus, playersInTick, bomb, grenadesInTick, hurtEventsInTick, deathEventsInTick );

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
