/*
 OpenLieroX
 
 gamemode interface
 
 created 2009-02-09
 code under LGPL
 */

#ifndef __CGAMEMODE_H__
#define __CGAMEMODE_H__

#include "CVec.h"
#include "Iterator.h"
#include "Consts.h"
#include <string>

class CWorm;
class GameServer;
class CServerConnection;

class CGameMode {
public:
	CGameMode() {}
	virtual ~CGameMode() {}

	// This is the game mode name as shown in the lobby for clients
	virtual std::string Name() = 0;
	
	virtual void PrepareGame() {}
	virtual void PrepareWorm(CWorm* worm) {}
	// If Spawn returns false then no spawn packet will be sent
	virtual bool Spawn(CWorm* worm, CVec pos);
	virtual void Kill(CWorm* victim, CWorm* killer) {}
	// If Shoot returns false then no shot will be fired
	virtual bool Shoot(CWorm* worm) { return true; }
	virtual void Drop(CWorm* worm) {}
	virtual void Carve(CWorm* worm, int d) {}
	virtual void Simulate() {}
	
	virtual bool CheckGameOver();
	virtual int  GeneralGameType(); // this is the game type which is sent over network
	virtual int  GameTeams();
	virtual std::string TeamName(int t);
	virtual int CompareWormsScore(CWorm* w1, CWorm* w2);
	virtual int CompareTeamsScore(int t1, int t2);
	virtual int  Winner();
	virtual int WinnerTeam();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm) { return true; }

	// helper functions
	int WormsAliveInTeam(int t);
	int TeamKills(int t);
	long TeamDamage(int t);
	int HighestScoredWorm();
	int HighestScoredTeam();
	
};

void InitGameModes();
CGameMode* GameMode(GameModeIndex i); // GM_* is parameter
CGameMode* GameMode(const std::string& name);
GameModeIndex GetGameModeIndex(CGameMode* gameMode);
Iterator<CGameMode* const&>::Ref GameModeIterator();

#endif

