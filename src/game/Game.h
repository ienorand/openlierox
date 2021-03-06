/*
 *  Game.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.12.09.
 *  code under LGPL
 *
 */

#ifndef __OLX_GAME_H__
#define __OLX_GAME_H__

#include <boost/signal.hpp>
#include <vector>
#include <list>

#include "olx-types.h"
#include "gusanos/object_grid.h"

class CWormHumanInputHandler;
class CWormInputHandler;
class CWorm;
class CMap;
class CGameScript;
class CGameMode;
class CWpnRest;

class Game {
public:
	void prepareGameloop();
	void frameOuter();
	void frameInner();
	void cleanupAfterGameloopEnd();
	
	void reset();
	
	void onNewWorm(CWorm* w);
	void onRemoveWorm(CWorm* w);
	void onNewPlayer(CWormInputHandler*);
	void onNewPlayer_Lua(CWormInputHandler*);
	void onNewHumanPlayer(CWormHumanInputHandler*);
	void onNewHumanPlayer_Lua(CWormHumanInputHandler*);
	
	bool isServer();
	bool isClient() { return !isServer(); }
	bool isTeamPlay();
	bool isGamePaused();
	bool shouldDoPhysicsFrame();
	
	bool needToCreateOwnWormInputHandlers();
	bool needProxyWormInputHandler();
	
	std::vector<CWormHumanInputHandler*> localPlayers;
	std::vector<CWormInputHandler*> players;
	
	Grid objects;

	CMap* gameMap();
	CGameScript* gameScript();
	CGameMode* gameMode();
	CWpnRest* weaponRestrictions();
	
	// they will be called in cleanupAfterGameloopEnd and the slot will be cleaned after that
	boost::signal<void()> cleanupCallbacks;
	
private:
	AbsTime oldtime;
};

extern Game game;

#endif
