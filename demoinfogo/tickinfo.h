#ifndef TICKINFO_H
#define TICKINFO_H

#include "enums.h"
#include "player.h"
#include <vector>

class TickInfo
{
public:
	TickInfo( int tickNumber, int roundNumber, RoundStatus roundStatus );
	~TickInfo();

	void AddPlayer( Player* player );
	void Print();

private:
	int tickNumber;
	int roundNumber;
	RoundStatus roundStatus;
	std::vector< Player > playersInTick;
};

#endif
