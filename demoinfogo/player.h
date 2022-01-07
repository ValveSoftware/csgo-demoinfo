#ifndef PLAYER_H
#define PLAYER_H

#include "enums.h"
#include <stdio.h>
#include <string>

class Player
{
public:
	/*Player( int entityID, int userID, const char* name, int teamID, 
		float x, float y, float z, float eyePitch, float eyeYaw, 
		int health, int armour, bool hasHelmet, bool hasDefuseKit,
		int money, float flashDuration );*/
	Player( int GUID, int entityID, int userID, std::string name, bool isBot );
	~Player();

	bool CheckBlind();
	bool CheckDead();
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

	void SetIsConnected ( bool isConnected );
	void SetStatus( PlayerStatus status );

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
	PlayerStatus status;
	//int kills;
	//int assists;
	//int deaths;
	//int killsCurrentRound;
	//weapons
	//headshots, damage, util damage, flashes?

private:
	int GUID;
	bool isConnected;
	bool isBot;
	std::string name;
};

#endif