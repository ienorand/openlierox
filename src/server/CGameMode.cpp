/*
 *  CGameMode.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.03.09.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "CServer.h"
#include "Options.h"
#include "CWorm.h"
#include "Protocol.h"

int CGameMode::GeneralGameType() {
	if(GameTeams() == 1)
		return GMT_NORMAL;
	else
		return GMT_TEAMS;
}


int CGameMode::GameTeams() {
	return 1;
}

std::string CGameMode::TeamName(int t) {
	static const std::string teamnames[4] = { "blue", "red", "green", "yellow" };
	if(t >= 0 && t < 4) return teamnames[t];
	return itoa(t);
}

bool CGameMode::CheckGameOver() {
	// In game?
	if (!cServer || cServer->getState() == SVS_LOBBY)
		return true;

	// Check if the timelimit has been reached
	if(tLXOptions->tGameInfo.fTimeLimit > 0) {
		if (cServer->getServerTime() > tLXOptions->tGameInfo.fTimeLimit*60.0) {
			if(networkTexts->sTimeLimit != "<none>")
				cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
			notes << "time limit (" << (tLXOptions->tGameInfo.fTimeLimit*60.0) << ") reached with current time " << cServer->getServerTime();
			notes << " -> game over" << endl;
			return true;
		}
	}
	
	if(!tLXOptions->tGameInfo.features[FT_AllowEmptyGames] || tLX->iGameType == GME_LOCAL) {
		// TODO: move that to GameServer
		int worms = 0;
		for(int i = 0; i < MAX_WORMS; i++)
			if(cServer->getWorms()[i].isUsed()) {
				if (cServer->getWorms()[i].getLives() != WRM_OUT)
					worms++;
			}

		if(worms <= (tLX->iGameType == GME_LOCAL ? 0 : 1)) {
			notes << "only " << worms << " players left in game -> game over" << endl;
			return true;
		}
	}

	if(GameTeams() > 1) {
		// Only one team left?
		int teams = 0;
		for(int i = 0; i < GameTeams(); i++)  {
			if(WormsAliveInTeam(i)) {
				teams++;
			}
		}
		
		// Only one team left
		if(teams <= 1) {
			return true;
		}
	}
	
	return false;
}

int CGameMode::Winner() {
	return HighestScoredWorm();
}

int CGameMode::HighestScoredWorm() {
	int wormid = -1;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed()) {
			if(wormid < 0) wormid = i;
			else
				wormid = CompareWormsScore(&cServer->getWorms()[i], &cServer->getWorms()[wormid]) > 0 ? i : wormid; // Take the worm with the best score
		}
	
	return wormid;
}

int CGameMode::CompareWormsScore(CWorm* w1, CWorm* w2) {
	// Lives first
	if(tLXOptions->tGameInfo.iLives >= 0) {
		if (w1->getLives() > w2->getLives()) return 1;
		if (w1->getLives() < w2->getLives()) return -1;		
	}
	
	// Kills
	if (w1->getKills() > w2->getKills()) return 1;
	if (w1->getKills() < w2->getKills()) return -1;
	
	// Damage
	return w1->getDamage() - w2->getDamage();
}

int CGameMode::WinnerTeam() {
	if(GameTeams() > 1)
		return HighestScoredTeam();
	else
		return -1;
}

int CGameMode::HighestScoredTeam() {
	int team = -1;
	for(int i = 0; i < GameTeams(); i++)
		if(team < 0) team = i;
		else
			team = CompareTeamsScore(i, team) > 0 ? i : team; // Take the team with the best score
	
	return team;
}

int CGameMode::WormsAliveInTeam(int t) {
	int c = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getLives() != WRM_OUT && cServer->getWorms()[i].getTeam() == t)
			c++;
	return c;
}

int CGameMode::TeamKills(int t) {
	int c = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getTeam() == t)
			c += cServer->getWorms()[i].getKills();
	return c;
}

long CGameMode::TeamDamage(int t) {
	long c = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getTeam() == t)
			c += cServer->getWorms()[i].getDamage();
	return c;
}


int CGameMode::CompareTeamsScore(int t1, int t2) {
	// Lives first
	if(tLXOptions->tGameInfo.iLives >= 0) {
		int d = WormsAliveInTeam(t1) - WormsAliveInTeam(t2);
		if(d != 0) return d;
	}
	
	// Kills
	{
		int d = TeamKills(t1) - TeamKills(t2);
		if(d != 0) return d;
	}
	
	// Damage
	return TeamDamage(t1) - TeamDamage(t2);
}



extern CGameMode* gameMode_DeathMatch;
extern CGameMode* gameMode_TeamDeathMatch;
extern CGameMode* gameMode_Tag;
extern CGameMode* gameMode_Demolitions;
extern CGameMode* gameMode_HideAndSeek;


static std::vector<CGameMode*> gameModes;

void InitGameModes() {
	gameModes.resize(5);
	gameModes[0] = gameMode_DeathMatch;
	gameModes[1] = gameMode_TeamDeathMatch;
	gameModes[2] = gameMode_Tag;
	gameModes[3] = gameMode_Demolitions;
	gameModes[4] = gameMode_HideAndSeek;	
}

CGameMode* GameMode(GameModeIndex i) {
	if(i < 0 || (uint)i >= gameModes.size()) {
		errors << "gamemode " << i << " requested" << endl;
		return NULL;
	}
	
	return gameModes[i];
}

CGameMode* GameMode(const std::string& name) {
	for(std::vector<CGameMode*>::iterator i = gameModes.begin(); i != gameModes.end(); ++i) {
		if(name == (*i)->Name())
			return *i;
	}
	return NULL;
}

GameModeIndex GetGameModeIndex(CGameMode* gameMode) {
	if(gameMode == NULL) return GM_DEATHMATCH;
	int index = 0;
	for(std::vector<CGameMode*>::iterator i = gameModes.begin(); i != gameModes.end(); ++i, ++index) {
		if(gameMode == *i)
			return (GameModeIndex)index;
	}
	return GM_DEATHMATCH;
}



Iterator<CGameMode* const&>::Ref GameModeIterator() {
	return GetConstIterator(gameModes);
}
