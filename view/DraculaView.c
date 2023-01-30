////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
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
#include <math.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Queue.h"

#define MAX 50

struct draculaView
{
	GameView gv;	 // Game View
	char *pastPlays; // Past Plays String
	int round;
	int pastPlaysSIZE;
	Map gameMap;
};

////////////////////////////////////////////////////////////////////////
bool hideMoveValid(DraculaView dv, char *pastPlays, Round round, int size); // Can dracula make a hide move? Yes: return true, No: return false
bool doubleBackMoveValid(char *pastPlays, Round round, int size);			// Can dracula make a double back move? Yes: return true, No: return false
bool hasBeenVisited(DraculaView dv, PlaceId location);						// Check if Dracula has visited a given location in his last 5 moves
PlaceId DraculaMoveID(DraculaView dv, int round);
bool isAdjacent(DraculaView dv, int doubleBackSize, PlaceId location);		// Given a location is a double back adjacent to the location
PlaceId doubleBackLocation(DraculaView dv, int doubleBackSize);
bool AdjacentToEndOfTrail(DraculaView dv, PlaceId nextLocation);
PlaceId *DvWhereCanIGoFromLoc(DraculaView dv, int *numReturnedLocs, PlaceId src);
PlaceId doubleBackLocation(DraculaView dv, int doubleBackSize);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL)
	{
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	new->gv = GvNew(pastPlays, messages);
	new->pastPlays = strdup(pastPlays);
	new->round = DvGetRound(new);
	new->gameMap = MapNew();
	new->pastPlaysSIZE = 0;
	for (int i = 0; pastPlays[i] != '\0'; i++)
		new->pastPlaysSIZE++;
	return new;
}

void DvFree(DraculaView dv)
{
	// Free past plays string
	free(dv->pastPlays);

	// Free map
	MapFree(dv->gameMap);

	// Free GameView
	GvFree(dv->gv);

	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv)
{
	return GvGetScore(dv->gv);
}

int DvGetHealth(DraculaView dv, Player player)
{
	return GvGetHealth(dv->gv, player);
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	return GvGetPlayerLocation(dv->gv, player);
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv->gv);
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	*numTraps = 0;
	return GvGetTrapLocations(dv->gv, numTraps);
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	int returnedMovesCounter = 0;
	int numReturnedLocations = 0;
	PlaceId *validMoves = malloc(MAX * sizeof(PlaceId)); // MAX is a magic number (randomly chosen cause numReturnedMoves will give an upper bound on the No. of moves)
	Round round = dv->round;
	if (round < 1)
	{
		free(validMoves);
		*numReturnedMoves = 0;
		return NULL;
	}																													 // Dracula hasn't made a move yet
	PlaceId currentLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA);												 // get Dracula's current location
	if (currentLocation == TELEPORT) currentLocation = CASTLE_DRACULA;
	PlaceId *reachableLocations = GvGetReachable(dv->gv, PLAYER_DRACULA, round, currentLocation, &numReturnedLocations); // get all reachable locations
	for (int i = 0; i < numReturnedLocations; i++)																		 // copy each location from reachable locations to validMoves (Assume that GvGetReachable returns REAL valid locations)
	{
		if (!hasBeenVisited(dv, reachableLocations[i]))
		{
			validMoves[returnedMovesCounter] = reachableLocations[i];
			returnedMovesCounter++;
		}
	}
	// If hide move is available add to valid moves array
	if (hideMoveValid(dv, dv->pastPlays, round, dv->pastPlaysSIZE))
	{
		validMoves[returnedMovesCounter] = HIDE;
		returnedMovesCounter++;
	}
	// If double back is available add all double backs that are less than the round size (i.e. can't have a double back of 5 when in round 2)
	if (doubleBackMoveValid(dv->pastPlays, round, dv->pastPlaysSIZE))
	{
		for (int i = round; i > 0; i--)
		{
			// if double back is adjacent add it
			if (isAdjacent(dv, i, currentLocation))
			{
				validMoves[returnedMovesCounter] = HIDE + i; // HIDE + i == double back place ID
				returnedMovesCounter++;
			}
		}
	}
	free(reachableLocations);
	if (returnedMovesCounter == 0)
	{
		free(validMoves);
		return NULL;
	}
	*numReturnedMoves = returnedMovesCounter;
	return validMoves;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	// getting all reachable locations (includes current location and locations in trail)
	PlaceId *reachable = DvWhereCanTheyGo(dv, PLAYER_DRACULA, numReturnedLocs);
	// copies every location over to another array except for the current location and locations in trail
	PlaceId *validPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int placeCount = 0;
	for (int i = 0; i < *numReturnedLocs; i++)
	{
		if (!hasBeenVisited(dv, reachable[i]))
		{
			validPlaces[placeCount] = reachable[i];
			placeCount++;
		}
	}
	*numReturnedLocs = placeCount;
	free(reachable);
	return validPlaces;
}

PlaceId *DvWhereCanIGoFromLoc(DraculaView dv, int *numReturnedLocs, PlaceId src)
{
	// getting all reachable locations (includes current location and locations in trail)
	PlaceId *reachable = GvGetReachable(dv->gv, PLAYER_DRACULA, 0, src, numReturnedLocs);
	// copies every location over to another array except for the current location and locations in trail
	PlaceId *validPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int placeCount = 0;
	for (int i = 0; i < *numReturnedLocs; i++)
	{
		if (!hasBeenVisited(dv, reachable[i]))
		{
			validPlaces[placeCount] = reachable[i];
			placeCount++;
		}
	}
	*numReturnedLocs = placeCount;
	free(reachable);
	return validPlaces;
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
							 int *numReturnedLocs)
{
	// getting all reachable locations by type (includes current location and locations in trail)
	PlaceId *reachable = DvWhereCanTheyGoByType(dv, PLAYER_DRACULA, road, false, boat, numReturnedLocs);
	// copies every location over to another array except for the current location and locations in trail
	PlaceId *validPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int placeCount = 0;
	for (int i = 0; i < *numReturnedLocs; i++)
	{
		if (!hasBeenVisited(dv, reachable[i]))
		{
			validPlaces[placeCount] = reachable[i];
			placeCount++;
		}
	}
	*numReturnedLocs = placeCount;
	free(reachable);
	return validPlaces;
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
						  int *numReturnedLocs)
{
	// getting all reachable locations (includes current location)
	PlaceId *reachable = GvGetReachable(dv->gv, player, dv->round, DvGetPlayerLocation(dv, player), numReturnedLocs);
	// copies every location over to another array except for the current location
	PlaceId *validPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int placeCount = 0;
	for (int i = 0; i < *numReturnedLocs; i++)
	{
		if (reachable[i] != DvGetPlayerLocation(dv, player))
		{
			validPlaces[placeCount] = reachable[i];
			placeCount++;
		}
	}
	*numReturnedLocs = placeCount;
	free(reachable);
	return validPlaces;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player, bool road, bool rail, bool boat,
								int *numReturnedLocs)
{
	// getting all reachable locations by given types (includes current location)
	PlaceId *reachable = GvGetReachableByType(dv->gv, player, dv->round, DvGetPlayerLocation(dv, player),
											  road, rail, boat, numReturnedLocs);
	// copies every location over to another array except for the current location
	PlaceId *validPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int placeCount = 0;
	for (int i = 0; i < *numReturnedLocs; i++)
	{
		if (reachable[i] != DvGetPlayerLocation(dv, player))
		{
			validPlaces[placeCount] = reachable[i];
			placeCount++;
		}
	}
	*numReturnedLocs = placeCount;
	free(reachable);
	return validPlaces;
}

// Assign a weighting to all locaitons (see bottom of .h file for more info)
int *mapWeightings(DraculaView dv)
{
	int *weightedLocs = malloc(NUM_REAL_PLACES * sizeof(double)); // Array of weighted locations (Index by Placeid, value at index is the weight)
	// Assign each location it's starting weight
	for (int i = 0; i < NUM_REAL_PLACES; i++)
		weightedLocs[i] = LOCATION_START_WEIGHTING;

	// Location loses weight points if at sea
	for (PlaceId curr = MIN_REAL_PLACE; curr <= MAX_REAL_PLACE; curr++)
	{ // for each place at sea reduce weight according to the following cost function
		if (placeIsSea(curr))
		{
			if (DvGetHealth(dv, PLAYER_DRACULA) <= GAME_START_BLOOD_POINTS-20) // cost function in dracula's health is less than or equal to 40
				weightedLocs[curr] -= ((-9 * DvGetHealth(dv, PLAYER_DRACULA)) + 200)/10;
			else if (DvGetHealth(dv, PLAYER_DRACULA) > GAME_START_BLOOD_POINTS) // reward function if dracula's health is greater than 40
				weightedLocs[curr] += ((9 * DvGetHealth(dv, PLAYER_DRACULA)) -329)/20;
			//if (weightedLocs[curr] < 0) weightedLocs[curr] = 0;
		}
	}

	// Give castle dracula a specific weight
	// The lower Dracula's health is his marginal benefit to get more blood points
	// will exceed is marginal cost to expose his location
	weightedLocs[CASTLE_DRACULA] -= LOCATION_IS_CASTLE_DRACULA * (DvGetHealth(dv, PLAYER_DRACULA) / 40);

	int numReturnedLocs = 0, numLocs = 0;
	PlaceId location;
	PlaceId *moves = DvGetValidMoves(dv, &numReturnedLocs);
	PlaceId *moveLocations = malloc(numReturnedLocs * sizeof(PlaceId));
	for (int i = 0; i < numReturnedLocs; i++) {
		if (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_5) {
			location = doubleBackLocation(dv, (moves[i]) - DOUBLE_BACK_1);
		} else if (moves[i] == HIDE) {
			location = doubleBackLocation(dv, 0);
		} else {
			location = moves[i];
		}
		bool inLocations = false; 
		for (int i = 0; i < numLocs; i++) {
			if (moveLocations[i] == location) inLocations = true;
		}
		if (!inLocations) { 
			moveLocations[numLocs] = location;
			numLocs++;
		}
	}

	for (int i = 0; i < numLocs; i++) { // loop through each location that dracula can move to
		int minDist = 100000; // large number such that minDist is larger that distance initially
		for (Player player = PLAYER_LORD_GODALMING; player <= PLAYER_MINA_HARKER; player++) {
			int distance = 0;
			GvGetShortestPathTo(dv->gv, player, moveLocations[i], &distance);
			minDist = (distance < minDist) ? distance:minDist; // if the current distance is shorter than minDist take that as the minimum distance
		}
		int weight = 0;
		//if (minDist == 0) weight = LOCATION_IS_REACHABLE_BY_HUNTER;
		if (!placeIsSea(moveLocations[i])) weight = round(exp((minDist - 7.0)/(-1.6)));
		weightedLocs[moveLocations[i]] -= weight;
		//printf("%s:Weight = %d, MinDist = %d\n", placeIdToName(moveLocations[i]), weight, minDist);
		
		// Get the number of locations for each reachable location & reward locations that have more locations
		int numLocations = 0;
		DvWhereCanIGoFromLoc(dv, &numLocations, moveLocations[i]);
		printf("location: %s, players near CD: %d, connections: %d\n", placeIdToName(moveLocations[i]),playersNearCD(dv, 3), numLocations);
		if ((numLocations == 0 && (playersNearCD(dv, 3) >= 2)) && !AdjacentToEndOfTrail(dv, moveLocations[i]))
		{ // if the reachable location will cause Dracula to teleport & if there are lots of hunters around CD then avoid teleporting
			weightedLocs[moveLocations[i]] -= LOCATION_IS_CASTLE_DRACULA;
		}
		else {weightedLocs[moveLocations[i]] += numLocations;} // reward locations for having more connections

	}

	return weightedLocs;
}

PlaceId *dvGetTrail(DraculaView dv, int *numReturnedLocs)
{
	bool canFree = true;
	return GvGetLastLocations(dv->gv, PLAYER_DRACULA, 5, numReturnedLocs, &canFree);
}

PlaceId *playermoveRadius(DraculaView dv, Player player, int radius, int *numReturnedLocations)
{
	PlaceId current = GvGetPlayerLocation(dv->gv, player);
	return moveRadius(dv->gv, player, current, radius, numReturnedLocations);
}

PlaceId *locationHistory(DraculaView dv, Player player, int *numReturnedLocs)
{
	bool canFree = true;
	return GvGetLocationHistory(dv->gv, player, numReturnedLocs, &canFree);
}

PlaceId *unreachables(DraculaView dv, int moves, int *numReturnedLocations) // CURRENTLY EXCLUDES SEAS
{
	PlaceId *locations = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	for (int i = 0; i < NUM_REAL_PLACES; i++)
		locations[i] = 1; // initialise visited array to all -1's

	int numPlayerReachables = 0;
	for (int i = 0; i < NUM_PLAYERS - 1; i++)
	{ // find all reachable locations for each player
		PlaceId *reachableLocs = playermoveRadius(dv, i, moves, &numPlayerReachables);
		for (int j = 0; j < numPlayerReachables; j++)
		{
			//printf("%d: %s\n", i, placeIdToName(reachableLocs[j]));
			locations[reachableLocs[j]] = -1;
		}
	}

	PlaceId *unreachables = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	*numReturnedLocations = 0;
	for (int i = 0; i < NUM_REAL_PLACES; i++)
	{
		if (locations[i] == 1 && placeIsSea(i) == false)
		{
			//printf("%s\n", placeIdToName(i));
			unreachables[*numReturnedLocations] = i;
			(*numReturnedLocations)++;
		}
	}
	free(locations);
	return unreachables;
}

int locationConnections(DraculaView dv, PlaceId src)
{
	int connections;
	GvGetReachable(dv->gv, PLAYER_DRACULA, 0, src, &connections);
	return connections - 1;
}

PlaceId *DvGetShortestPathTo(DraculaView dv, Player player, PlaceId dest, int *pathLength)
{
	return GvGetShortestPathTo(dv->gv, player, dest, pathLength);
}

PlaceId *DvGetSafestPathTo(DraculaView dv, Player player, PlaceId dest, int *pathLength)
{
	PlaceId *path = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	PlaceId src = DvGetPlayerLocation(dv, player); // Gets current hunters location
	PlaceId *visited = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	for (int i = 0; i < NUM_REAL_PLACES; i++) visited[i] = -1;    // initialise visited array to all -1's

	int found = 0;
	visited[src] = src;     // mark src as visited and add to queue
	Queue Q = newQueue();
	QueueJoin(Q, src);
	
	while (!found && QueueIsEmpty(Q) == 0) {
	    PlaceId v = QueueLeave(Q);

		int i = v; // calculate current round by traversing visited array
		int round = dv->round;
		while (i != src) {
			i = visited[i]; 
			round++;
		}

	    if (v == dest) {
			found = 1;
	    } else {    // visit neighbor vertices
			int numReturnedLocs = 0;	// Get all (valid) connections to that city
	        PlaceId *connections = GvGetReachableByType(dv->gv, player, round, v, true, false, false, &numReturnedLocs);
			sortPlaces(connections, numReturnedLocs);
	        for (int i = 0; i < numReturnedLocs; i++) {
	            if (visited[connections[i]] == -1) {
	                visited[connections[i]] = v;
	                QueueJoin(Q, connections[i]);
	            }
	        }
			free(connections);
	    }
	}

	if (found) {
	    // store path in reverse order in a temporary array
	    PlaceId temp[NUM_REAL_PLACES], len = 0, i = dest;
	    while (i != src) {
	        temp[len] = i;
	        i = visited[i]; 	 
	        len++;   
	    }
	    
	    // invert path for output
	    int k = 0;
	    for (int i = len - 1; i >= 0; i--) {
	        path[k] = temp[i];
	        k++;
	    }
	    *pathLength = len;
	}
	free(visited);
	dropQueue(Q);
	return path;
}

int playersNearCD(DraculaView dv, int round)
{
	int near = 0;
	for (int i = 0; i < NUM_PLAYERS - 1; i++) {
		int lengthToCD = 0;
		DvGetShortestPathTo(dv, i, CASTLE_DRACULA, &lengthToCD);
		if (lengthToCD <= round) {
			near++;
		}
	}
	return near;
}

////////////////////////////////////////////////////////////////////////
//						Begin Utility Functions                       //
////////////////////////////////////////////////////////////////////////

// Can dracula make a hide move? Yes: return true, No: return false
bool hideMoveValid(DraculaView dv, char *pastPlays, Round round, int size)
{
	if (placeIsSea((DvGetPlayerLocation(dv, PLAYER_DRACULA)))) return false; // Dracula cannot hide while at sea
	for (int i = round; i >= round - 4; i--) // check last 5 rounds
	{
		int draculaLOC = getPlayer(i, PLAYER_DRACULA);
		if (draculaLOC > size || draculaLOC < 0)
			continue;
		if (pastPlays[draculaLOC + 1] == 'H' && pastPlays[draculaLOC + 2] == 'I')
			return false; // return false is a hide move is in Dracula's location history
	}
	return true;
}

// Can dracula make a double back move? Yes: return t
bool doubleBackMoveValid(char *pastPlays, Round round, int size)
{
	char abbrev[3];
	abbrev[2] = '\0';
	for (int i = round; i >= round - 4; i--) // check last 5 rounds
	{
		int draculaLOC = getPlayer(i, PLAYER_DRACULA);
		if (draculaLOC > size || draculaLOC < 0) // this bounds the array so that an index outside of the array isn't accessed
			continue;
		abbrev[0] = pastPlays[draculaLOC + 1];
		abbrev[1] = pastPlays[draculaLOC + 2];
		if (placeAbbrevToId(abbrev) >= DOUBLE_BACK_1 && placeAbbrevToId(abbrev) <= DOUBLE_BACK_5)
			return false; // return false if a double back move has been made
	}
	return true;
}

// Check if Dracula has visited a given location in his last 6 moves
// excludes locations that he visited in his trail ONLY via double back or hide
bool hasBeenVisited(DraculaView dv, PlaceId location)
{
	int numReturnedLocs = 0;
	bool canFree = false;

	if (location == DvGetPlayerLocation(dv, PLAYER_DRACULA)) return true; //! Quick fix: Problem is that without this dracula will try to move back to his current location

	PlaceId *trail = GvGetLastLocations(dv->gv, PLAYER_DRACULA, TRAIL_SIZE-1, &numReturnedLocs, &canFree); // gets all locations in his trail

	PlaceId *specialMoveLocs = malloc(numReturnedLocs * sizeof(PlaceId)); // array of locations which he has reached through a hide move or double back
	int specialMoves = 0;
	for (int i = 0; i < numReturnedLocs; i++) { // loop through and add locations that have been visited through hiding or double backing
		PlaceId move = DraculaMoveID(dv, dv->round - i); // uses helper function
		if (!placeIsReal(move) && move != TELEPORT) {
			specialMoveLocs[specialMoves] = trail[i];
			specialMoves++;
		}
	}

	int blacklisted = 0;
	// blacklisted locations from the trail, ie. the location hasn't been visited by a normal move in the last 6 turns and can thus be revisited
	PlaceId *blacklist = malloc(numReturnedLocs * sizeof(PlaceId));
	for (int i = 0; i < specialMoves; i++) { // loop through locations in his trail visited by double back or hide
		bool ValidMoveFound = false;
		for (int j = 0; j < numReturnedLocs; j++) { // loop through locations in the trail
			if (trail[j] == specialMoveLocs[i] && placeIsReal(DraculaMoveID(dv, dv->round - j))) { // if the location has been visited by a double back or hide AND by a normal move to that location
				ValidMoveFound = true;
			}
		}
		if (!ValidMoveFound) { // if not found, then he can revisit that location this turn
			blacklist[blacklisted] = specialMoveLocs[i];
			blacklisted++;
		}
	}

	int trailSize = 0;
	PlaceId *newTrail = malloc(numReturnedLocs * sizeof(PlaceId)); // new updated trail without the blacklisted locations
	for (int i = 0; i < numReturnedLocs; i++) { // loop through trail and add locations that are not in the blacklisted array
		bool BlacklistFound = false;
		for (int j = 0; j < blacklisted; j++) {
			if (trail[i] == blacklist[j]) {
				BlacklistFound = true;
			}
		}
		if (!BlacklistFound) {
			newTrail[trailSize] = trail[i];
			trailSize++;
		}
	}
	free(trail);
	free(blacklist);
	free(specialMoveLocs);

	for (int i = 0; i < trailSize; i++)
	{
		if (location == newTrail[i])
		{
			free(newTrail);
			return true;
		}
	}
	free(newTrail);
	return false;
}

// get dracula's move, either returns the location he was at or double back, hide
PlaceId DraculaMoveID(DraculaView dv, int round) // helper function for hasBeenVisited
{
	char abbrev[3];
	abbrev[2] = '\0';
	char temp = dv->pastPlays[getPlayer(round, PLAYER_DRACULA) + 1]; // first letter of move abbrev
	abbrev[0] = temp;
	temp = dv->pastPlays[getPlayer(round, PLAYER_DRACULA) + 2]; // second letter of move abbrev
	abbrev[1] = temp;
	return placeAbbrevToId(abbrev);
}

// Given a location is a double back adjacent to the location
bool isAdjacent(DraculaView dv, int doubleBackSize, PlaceId location)
{
	int numReturnedLocs = 0;
	int numConns = 0;
	bool canFree = false;
	PlaceId *trail = GvGetLocationHistory(dv->gv, PLAYER_DRACULA, &numReturnedLocs, &canFree); // get dracula's trail
	if (doubleBackSize > numReturnedLocs || doubleBackSize >= 6)
	{
		free(trail);
		return false;
	}																	  // invalid array index, and illegal double back size (respectively)
	PlaceId doubleBackLocation = trail[numReturnedLocs - doubleBackSize]; // get the location to which the double back refers to
	PlaceId *connections = DvWhereCanTheyGo(dv, PLAYER_DRACULA, &numConns); // get a list of adjacent cities to the given location

	if (doubleBackLocation == location)
	{
		free(trail);
		return true; // double back location is the location (occurs if double back size == 1 & round = 1)
	}
	for (int i = 0; i < numConns; i++) { // search through all connected cities
		if (connections[i] == doubleBackLocation) {
			free(trail);
			return true; // if the locaiton the double back refers to is a connection of the given city return true (ie. it is adjacent)
		}
	}
	free(trail);
	return false; // otherwise return false
}

// gets the last location in the trail
bool AdjacentToEndOfTrail(DraculaView dv, PlaceId nextLocation)
{
	int numReturnedLocs = 0, numConns = 0, reachable = 0;
	bool canFree = true;
	PlaceId *trail = GvGetLastLocations(dv->gv, PLAYER_DRACULA, TRAIL_SIZE-1, &numReturnedLocs, &canFree); // gets all locations in his trail
	if (numReturnedLocs == 0) { // if there is no trail
		free(trail);
		return false;
	}
	PlaceId *connections = GvGetReachable(dv->gv, PLAYER_DRACULA, dv->round + 1, nextLocation, &reachable); // get a list of adjacent cities to the given location
	for (int i = 0; i < numConns; i++) { // check if the given location is in the connections array
		if (connections[i] == trail[numReturnedLocs-1]) {
			free(connections);
			free(trail);
			return true;
		}
	}
	free(connections);
	free(trail);
	return false;
}

// returns the double back location for a given double back size, if size exceeds size of trail returns NOWHERE
PlaceId doubleBackLocation(DraculaView dv, int doubleBackSize)
{
	int numReturnedLocs = 0;
	PlaceId *trail = dvGetTrail(dv, &numReturnedLocs); // get dracula's trail
	if (doubleBackSize > numReturnedLocs || doubleBackSize > 6)
	{
		free(trail);
		return NOWHERE;
	}
	PlaceId doubleBackLocation = trail[doubleBackSize];

	free(trail);
	return doubleBackLocation;
}
