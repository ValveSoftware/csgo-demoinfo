#ifndef PLAYER_H
#define PLAYER_H

#include "enums.h"
#include <stdio.h>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class Player
{
public:
	Player();
	Player( int GUID, int entityID, int userID, std::string name, bool isBot );
	Player( Player* player );
	~Player();

	bool CheckBlind();
	bool CheckDead();
	void TickCleanUp();
	void RoundCleanUp();
	void Print();

	int GetGUID();
	bool GetIsConnected();
	bool GetIsBot();
	int GetEntityID();
	int GetUserID();
	std::string GetName();
	int GetTeam();
	float GetX();
	float GetY();
	float GetZ();
	float GetEyePitch();
	float GetEyeYaw();
	int GetHealth();
	int GetArmour();
	bool GetHasHelmet();
	bool GetHasDefuseKit();
	int GetMoney();
	float GetFlashDuration();
	PlayerStatus GetStatus();
	bool GetHasBomb();

	void SetIsConnected ( bool isConnected );
	void SetStatus( PlayerStatus status );
	void SetHasBomb( bool hasBomb );

	int entityID;
	int userID;
	int teamID;
	float x;
	float y;
	float z;
	float eyePitch;
	float eyeYaw;
	int health;
	int armour;
	bool hasHelmet;
	bool hasDefuseKit;
	int money;
	float flashDuration;
	bool hasBomb;
	//int kills;
	//int assists;
	//int deaths;
	//int killsCurrentRound;
	//weapons
	//headshots, damage, util damage, flashes?

	NLOHMANN_DEFINE_TYPE_INTRUSIVE( Player, GUID, isConnected, isBot, entityID, userID, name, teamID, x, y, z, eyePitch, eyeYaw, health, armour, hasHelmet, hasDefuseKit, money, flashDuration, status, hasBomb );

private:
	int GUID;
	bool isConnected;
	bool isBot;
	std::string name;
	PlayerStatus status;
};

#endif