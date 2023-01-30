////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////
// fury-of-dracula-WALJ
// Julian Garratt (z5308427) - H15B - MillenniumForce
// William Dahl (z5317148) - W15B - William-Dahl
// Lachlan Rolley (z5162440) - W15B - LachlanRolley
// Adrian Tran (z5263781) - H15B - ShiniSaba
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Queue.h"
#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h"
#include "Places.h"

struct hunterView {
	GameView gv;
	char *pastPlays;
	Round round;
	Map gameMap;
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[])
{

	HunterView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
		exit(EXIT_FAILURE);
	}

	// Initialise all variables
	new->gv = GvNew(pastPlays, messages);
	new->pastPlays = strdup(pastPlays);
	new->round = HvGetRound(new);
	new->gameMap = MapNew();

	return new;
}

void HvFree(HunterView hv)
{
	// Frees Hunter gameview
	GvFree(hv->gv);

	// Frees PastPlays string
	free(hv->pastPlays);

	// Free Map
	MapFree(hv->gameMap);

	// Frees HunterView
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv)
{
	return GvGetRound(hv->gv);
}

Player HvGetPlayer(HunterView hv)
{
	return GvGetPlayer(hv->gv);
}

int HvGetScore(HunterView hv)
{
	return GvGetScore(hv->gv);
}

int HvGetHealth(HunterView hv, Player player)
{
	return GvGetHealth(hv->gv, player);
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player)
{
	return GvGetPlayerLocation(hv->gv, player);
}

PlaceId HvGetVampireLocation(HunterView hv)
{
	return GvGetVampireLocation(hv->gv);
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round)
{
	int numMoves = 0; *round = 0; bool canFree = false;
	PlaceId *locationHistory = GvGetMoveHistory(hv->gv, PLAYER_DRACULA, &numMoves, &canFree); // Get move history
	for (int i = numMoves-1; i >= 0; i--)
	{
		if (locationHistory[i] == TELEPORT) { // if location is teleport then set location to CD
			*round = i;
			PlaceId lastLocation = CASTLE_DRACULA;
			free(locationHistory);
			return lastLocation; // return location as CD

		}
		else if (placeIsReal(locationHistory[i]))
		{ // if the place is real return the corresponding placeID
			*round = i;
			PlaceId lastLocation = locationHistory[i];
			free(locationHistory);
			return lastLocation;
		}
	}
	free(locationHistory);
	return NOWHERE;
}

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest, int *pathLength)
{
	return GvGetShortestPathTo(hv->gv, hunter, dest, pathLength);
}

PlaceId *HvplayermoveRadius(HunterView hv, Player player, PlaceId location, int radius, int *numReturnedLocations)
{
	return moveRadius(hv->gv, player, location, radius, numReturnedLocations);
}

PlaceId *HvlocationHistory(HunterView hv, Player player, int *numReturnedLocs)
{
	bool canFree = true;
	return GvGetLocationHistory(hv->gv, player, numReturnedLocs, &canFree);
}

bool isConnected(HunterView hv, PlaceId src, PlaceId location)
{
	int numReturnedLocations = 0;
	PlaceId *connections = GvGetReachable(hv->gv, HvGetPlayer(hv), hv->round, src, &numReturnedLocations);
	for (int i = 0; i < numReturnedLocations; i++)
	{ // search through all reachable locations
		if (connections[i] == src) {
			free(connections);return true;
		} // if one of the connected locations equals the source location then it is connected
	}
	free(connections);
	return false;
}
////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs)
{

	*numReturnedLocs = 0;

	Round round= GvGetRound(hv->gv); // get round
	Player player = GvGetPlayer(hv->gv); // get current player
	PlaceId from = GvGetPlayerLocation(hv->gv, player); // get the player's locations

	PlaceId *reachableLocations = GvGetReachable(hv->gv, player, round,
		from, numReturnedLocs); // get all reachable locations
	return reachableLocations; // return all reachable locations
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
	bool boat, int *numReturnedLocs)
{

	*numReturnedLocs = 0;
	Round round= GvGetRound(hv->gv); // get round
	Player player = GvGetPlayer(hv->gv); // get current player
	PlaceId from = GvGetPlayerLocation(hv->gv, player); // get player's location
	PlaceId *reachableLocations = GvGetReachableByType(hv->gv, player, round,
		from, road, rail,
		boat, numReturnedLocs);
	return reachableLocations; // return reachable locations by type
}

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
	int *numReturnedLocs)
{
	*numReturnedLocs = 0;
	Round round= GvGetRound(hv->gv); // get round
	Player currentPlayer = GvGetPlayer(hv->gv); // get current player
	PlaceId from = GvGetPlayerLocation(hv->gv, player);
	if (player < currentPlayer) round++; // if the given player has gone already (i.e player < current player) there next term will be in the next round

	PlaceId *reachableLocations = GvGetReachable(hv->gv, player, round,
		from, numReturnedLocs);
	return reachableLocations; // return reachable loations
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
	bool road, bool rail, bool boat,
	int *numReturnedLocs)
{
	*numReturnedLocs = 0;
	Round round= GvGetRound(hv->gv);
	Player currentPlayer = GvGetPlayer(hv->gv);
	PlaceId from = GvGetPlayerLocation(hv->gv, player);
	if (player <= currentPlayer) round++; // if the given player has gone already (i.e player < current player) there next term will be in the next round
	PlaceId *reachableLocations = GvGetReachableByType(hv->gv, player, round,
		from, road, rail,
		boat, numReturnedLocs);
	return reachableLocations; // return all reachable locations by type
}

////////////////////////////////////////////////////////////////////////