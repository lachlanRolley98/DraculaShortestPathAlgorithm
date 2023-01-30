////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
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

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"
#include "HunterView.h"
#include "DraculaView.h"
#include "Queue.h"

struct playerData
{
	Player currentPlayer;				// enum representing the current player as as number i.e. Dracula == 4 (starts at 0)
	int playerHealth[NUM_PLAYERS];		// array that stores the health of each character (Array size = 5, should be from 0-4 the same indexing used in the currentPlayer enum)
	PlaceId playerLoc[NUM_PLAYERS]; 	// array of places that stores each characters location (Array size = 5, should be from 0-4 the same indexing used in the currentPlayer enum)
	PlaceId draculaTrail[TRAIL_SIZE];
};

struct encounterData
{
	PlaceId locDestroyedVamp;			// location of destroyed immature vampire, initialised as nowhere
	int numDestroyedTraps;				// Number of destroyed traps, initialised as 0
	PlaceId *locDestroyedTraps; 		// Locations of all destroyed traps (dynamically allocated as size of array is variable)
};

struct gameView
{
	Map GameMap;						 // Graph ADT used to interact with Map.h
	Round round;						 // current round No. (known by the size of pastPlays char)
	int numPlays;						 // number of plays made i.e. 5 plays per round
	int score;							 // current score (I think this is managed by our gameview)
	char *pastPlays;					 // string of past plays
	int pastPlaysSIZE;					 // size of pastPlays array
	PlayerData pd;						 // Stores all info about players
	int weightedPlaces[NUM_REAL_PLACES]; // array assigning a weight >= 0 to each location, the higher the better
	EncounterData ed;					 // Stores all info about encounters
};
////////////////////////////////////////////////////////////////////////
// Additional helper functions
int getPlayer(int round, Player player);			// get a player's position in the array for the nth round (from round >= 1)
void initPlayers(GameView gv);						// initialise all players healths
PlaceId getTrail(GameView gv, int position);		// get dracula's last location in the trail at a given position
void addToTrail(GameView gv, PlaceId location);		// add dracula's most recent location to his trail
PlaceId trapIsDestroyed(GameView gv, PlaceId trap); // check if the trap is destroyed if so return true
PlaceId *GetLastPlaces(GameView gv, Player player, PlaceId *placeHistory, 
						int numPlaces, int *numReturnedPlaces, bool *canFree); // return the last 'numPlaces' PlaceId's in placeHistory array 
void printTrail(GameView gv); 						// print dracula's trail, useful for debugging
int getRailLocations(GameView gv, int railScore, PlaceId src, PlaceId *locations, int low, int size); // Given a locations array, add all locations that can be travelled to by rail
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[])
{
	// * Initialise Base Variables
	GameView new = malloc(sizeof(*new)); 			// *needs to be freed
	if (new == NULL)
	{
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	new->GameMap = MapNew();						//* needs to be freed
	new->round = 0;
	new->numPlays = 0;								// count of the number of plays i.e. 5 plays per round
	new->score = GAME_START_SCORE;
	new->pastPlays = strdup(pastPlays);				//* needs to be freed
	new->pd = malloc(sizeof(struct playerData)); 	//* needs to be freed
	new->pd->currentPlayer = 0;						// initialize as Lord Goldaming
	for (int i = 0; i < TRAIL_SIZE; i++)
		new->pd->draculaTrail[i] = NOWHERE;			// initialise Dracula's trail to NOWHERE
	new->ed = malloc(sizeof(struct encounterData)); //* needs to be freed
	new->ed->locDestroyedVamp = NOWHERE;
	new->ed->numDestroyedTraps = 0;

	// *Initialise Size of the array
	new->pastPlaysSIZE = 0; 						// pastPlays size starts as 0
	int numSpaces = 0;
	for (int i = 0; pastPlays[i] != '\0'; i++)
	{
		new->pastPlaysSIZE++; 						// initialise pastPlaysSize
		if (pastPlays[i] == ' ')
			numSpaces++;
	}
	// *Initialise numPlays and Round
	new->numPlays = (new->pastPlaysSIZE - numSpaces) / 7; // remove spaces from past plays array size, divide by seven to get num plays
	new->round = new->numPlays / 5;					// 5 plays per round therefore divide by 5 to get the number of rounds

	// *Initialise number of destroyed traps
	if (new->round >= 1)
	{
		new->ed->locDestroyedTraps = malloc(new->round * sizeof(PlaceId));
		for (int i = 0; i < new->round; i++)
			new->ed->locDestroyedTraps[i] = NOWHERE;
	}
	else new->ed->locDestroyedTraps = NULL;			// set loc destroyed traps to NULL

	initPlayers(new);								// *Initialise players with starting health and current locations

	return new;
}

void GvFree(GameView gv)
{
	// Free the Map
	MapFree(gv->GameMap);

	//Free past plays string
	free(gv->pastPlays);

	// Free Player data struct
	free(gv->pd);

	// Free encounter data struct
	if (gv->ed->locDestroyedTraps != NULL)
		free(gv->ed->locDestroyedTraps);
	free(gv->ed);

	// Free GameView
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	return gv->round;
}

Player GvGetPlayer(GameView gv)
{
	gv->pd->currentPlayer = (gv->numPlays % 5);
	return gv->pd->currentPlayer;
}

int GvGetScore(GameView gv)
{
	// Get the number times dracula has made a move (i.e. proportional to the No. of rounds)
	gv->score -= gv->round; // 1 score point lost per round

	// Test if Vampire Matured
	for (int i = 1; i <= gv->round; i++)
	{
		int pos = getPlayer(i, PLAYER_DRACULA);		 // get dracula's location in the array for the ith round
		if (gv->pastPlays[pos + 5] == 'V')			 // matured vampire is in position 5 + 1 of dracula's move (1 is 'D' denoting Dracula)
			gv->score -= SCORE_LOSS_VAMPIRE_MATURES; // Key for vampire maturing is always in the 5th position
	}

	return gv->score;
}

int GvGetHealth(GameView gv, Player player)
{
	if (gv->pd->playerHealth[player] < 0)
		gv->pd->playerHealth[player] = 0;
	return gv->pd->playerHealth[player];
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	if (gv->pd->playerHealth[player] <= 0)
		gv->pd->playerLoc[player] = ST_JOSEPH_AND_ST_MARY;
	return gv->pd->playerLoc[player];
}

PlaceId GvGetVampireLocation(GameView gv)
{
	PlaceId vampireLOC = NOWHERE;
	if (gv->ed->locDestroyedVamp != NOWHERE)
		return NOWHERE;								// vamp has been destroyed so report it's nonexistence
	for (int round = 1; round <= gv->round; round++) // dracula's past plays don't appear till beginning of round 1
	{
		int dracLOC = getPlayer(round, PLAYER_DRACULA);
		if (gv->pastPlays[dracLOC + 4] == 'V') 		// new vampire is in position 4 + 1 of dracula's move (1 is 'D' denoting Dracula)
		{
			char abbrev[3];
			abbrev[2] = '\0';
			char temp = gv->pastPlays[dracLOC + 1]; // first letter of location abbrev
			abbrev[0] = temp;
			temp = gv->pastPlays[dracLOC + 2]; 		// second letter of location abbrev
			abbrev[1] = temp;
			vampireLOC = placeAbbrevToId(abbrev); 	// this could return an unknown location (still valid)
		}
		if (gv->pastPlays[dracLOC + 5] == 'V')
			return NOWHERE; // Vampire has matured
	}

	return vampireLOC;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps) // This function only works at its best when player is a hunter
{
	gv->pd->currentPlayer = GvGetPlayer(gv);
	if (gv->pd->currentPlayer != PLAYER_DRACULA)
	{
		*numTraps = gv->ed->numDestroyedTraps; 	// hunter's don't have perfect information
		return gv->ed->locDestroyedTraps;	   	// !warning this could be NULL Hunter should check if it is NULL meaning no traps exist
	}
	else
	{
		if (gv->round < 1) return NULL;	// if round is less than 1 Dracula hasn't even made a move yet
		PlaceId *traps = malloc(gv->round * sizeof(PlaceId)); // Dracula can't physically place more traps then there are rounds
		for (int i = 0; i < gv->round; i++)
			traps[i] = NOWHERE; // initialise all places to nowhere
		int i = 0; // keeps track of where in the array the trap location should be placed
		for (int round = 1; round <= gv->round; round++)
		{
			int dracLOC = getPlayer(round, PLAYER_DRACULA);
			if (gv->pastPlays[dracLOC + 3] == 'T')
			{
				*numTraps += 1;
				char abbrev[3];
				abbrev[2] = '\0';
				char temp = gv->pastPlays[dracLOC + 1]; // first letter of location abbrev
				abbrev[0] = temp;
				temp = gv->pastPlays[dracLOC + 2]; // second letter of location abbrev
				abbrev[1] = temp;
				PlaceId newPlace = placeAbbrevToId(abbrev);
				if (trapIsDestroyed(gv, newPlace)) // check if the trap is still active
				{
					*numTraps -= 1;
					continue;
				}
				else if (newPlace == HIDE) // Dracula places a trap at his previous location
				{
					newPlace = getTrail(gv, 1); // Dracula goes back one position in his trail
				}
				else if (newPlace >= DOUBLE_BACK_1 && newPlace <= DOUBLE_BACK_5)
				{ // dracula places a trap at 1,2,3,4 or 5 locations back in his trail
					newPlace = getTrail(gv, abbrev[1]);
				}
				traps[i] = newPlace; // else trap is active add to list of traps
				i++;
			}
		}
		return traps;
	}
}

////////////////////////////////////////////////////////////////////////
// Game History

// outputs array of placeIds for all moves made by a player
PlaceId *GvGetMoveHistory(GameView gv, Player player,
						  int *numReturnedMoves, bool *canFree)
{
	PlaceId *moveHistory = malloc((gv->round + 1) * sizeof(PlaceId));
	int playerIndex, moveIndex = 0;
	char move[3];
	move[2] = '\0';
	for (int i = 1; i <= gv->round + 1; i++) { 	// loop through each round
		playerIndex = getPlayer(i, player);
		if (playerIndex > gv->pastPlaysSIZE) break; 
		move[0] = gv->pastPlays[playerIndex+1]; // extract player's move
		move[1] = gv->pastPlays[playerIndex+2];
		if (move[0] != '\0') { 					// check that move has been made
			moveHistory[moveIndex] = placeAbbrevToId(move);
			moveIndex++;
		}
	}
	*numReturnedMoves = moveIndex;
	*canFree = true;
	return moveHistory;
}

// outputs array of placeIds for the last 'numMoves' moves done by a player (see .h)
// in order of most recent to least, ie output[0] = last round's move
// NOTE: this calls the helper function GetLastPlaces(), which does all of it's work
PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
						int *numReturnedMoves, bool *canFree)
{
	PlaceId *moveHistory = GvGetMoveHistory(gv, player, numReturnedMoves, canFree);
	PlaceId *lastMoves = GetLastPlaces(gv, player, moveHistory, numMoves, numReturnedMoves, canFree); // use helper function
	free(moveHistory);
	return lastMoves;
}

// returns array of locations the player has visited
// essentially just filtering moveHistory to show locations based on dracula's moves
PlaceId *GvGetLocationHistory(GameView gv, Player player,
							  int *numReturnedLocs, bool *canFree)
{
	PlaceId *moveHistory = GvGetMoveHistory(gv, player, numReturnedLocs, canFree);
	if (player != PLAYER_DRACULA) return moveHistory; // if the player is hunter return moveHistory 
	PlaceId *locationHistory = malloc((*numReturnedLocs) * sizeof(PlaceId)); // number of locations can't exceed number of moves
	int locationCount = 0;
	for (int i = 0; i < *numReturnedLocs; i++) { // loop through moves
		if (moveHistory[i] < 102) { // move is a real place (no special move used)
			locationHistory[locationCount] = moveHistory[i];
		} else if (moveHistory[i] >= DOUBLE_BACK_1 && moveHistory[i] <= DOUBLE_BACK_5) { // Dracula made a double back
			locationHistory[locationCount] = locationHistory[locationCount-atoi(&placeIdToAbbrev(moveHistory[i])[1])];
		} else if (moveHistory[i] == HIDE) { // Dracula hid
			locationHistory[locationCount] = locationHistory[locationCount-1];
		} else if (moveHistory[i] == TELEPORT) { // Dracula teleported
			locationHistory[locationCount] = CASTLE_DRACULA;
		} else continue; // just incase for some reason the move doesn't pass one of the above if statements
		locationCount++;
	}
	free(moveHistory);
	*numReturnedLocs = locationCount;
	*canFree = true;
	return locationHistory;
}

// outputs array of placeIds for the last 'numLocs' locations visited by a player (see .h)
// in order of most recent to least, ie output[0] = last round's location
// NOTE: this calls the helper function GetLastPlaces(), which does all of the work
PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
							int *numReturnedLocs, bool *canFree)
{
	PlaceId *locationHistory = GvGetLocationHistory(gv, player, numReturnedLocs, canFree);
	PlaceId *lastLocations = GetLastPlaces(gv, player, locationHistory, numLocs, numReturnedLocs, canFree); // call helper function
	free(locationHistory);
	return lastLocations;
}

////////////////////////////////////////////////////////////////////////
// Making a Move
// Map.h is going to very useful for this

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
						PlaceId from, int *numReturnedLocs)
{
	// map.h contains MapGetConnections, this will return a linked list of everything connected to the place
	// runs through the linked list and adds it to the array
	// needs to do more when there are rail connections due to railscores
	*numReturnedLocs = 0;
	PlaceId *ReachablePlaces = malloc(sizeof(PlaceId) * 40); // players won't ever be able to go to access than 40 places in 1 turn
	for (int i = 0; i < 40; i++) ReachablePlaces[i] = NOWHERE; 
	ReachablePlaces[0] = from; // you can always get to where u are 
	ConnList head = MapGetConnections(gv->GameMap, from); // linked list of places we can immediately reach
	ConnList curr = head;
	if(head == NULL) return NULL;

	// the main loop;
	int i = 1; // start at one as src location has been added
	while(curr != NULL) {              
		                  
		if (player != PLAYER_DRACULA) // player is a hunter
		{
			if (curr->type == RAIL) // special case for rail type move 
			{
				int railScore = (round + player) % 4; // as defined in the spec 
				if (railScore >= 1) {
					bool visited = false; 
					for (int x = 0; x < i; x++) // check if location has been visited
					{
						if (ReachablePlaces[x] == curr->p) {visited = true; break;}
					}
					if (visited == false) {ReachablePlaces[i] = curr->p; i++;}
					i = getRailLocations(gv, railScore, curr->p, ReachablePlaces, i, 40); // get all rail locations recursively
				}
			}
			else {
				bool visited = false; 
				for (int x = 0; x < i; x++) // check if location has been visited
					{
						if (ReachablePlaces[x] == curr->p) {visited = true; break;}
					}
					if (visited == false) {ReachablePlaces[i] = curr->p; i++;}
			}
		}

		if (player == PLAYER_DRACULA) // player is Dracula 
		{
			// test if location is hospital, or by rail
			if (curr->type != RAIL && curr->p != ST_JOSEPH_AND_ST_MARY) 
			{
				ReachablePlaces[i] = curr->p;
				i++;
			}
		}
		
		curr = curr->next;
	}                               
	*numReturnedLocs = i ;
	return ReachablePlaces;
}

// same as GvGetReachable but limits it by connection type
PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
							  PlaceId from, bool road, bool rail,
							  bool boat, int *numReturnedLocs)
{
	*numReturnedLocs = 0;
	PlaceId *ReachablePlaces = malloc(sizeof(PlaceId)*40); 
	for (int i = 0; i < 40; i++) ReachablePlaces[i] = NOWHERE; 
	ReachablePlaces[0] = from; // you can always get to where u are 
	ConnList head = MapGetConnections(gv->GameMap, from); // now we are just gona have a linked list of places we can imediatly reach
	ConnList curr = head;
	
	if(head == NULL) return NULL;

	// the main loop;
	int i = 1;
	while(curr != NULL){              
		if(road ){
			if(curr->type == ROAD){
				if(player == PLAYER_DRACULA && curr->p == ST_JOSEPH_AND_ST_MARY){
					curr = curr->next;
					continue;
						
				}	
				ReachablePlaces[i]= curr->p; 
				i++;

			}
		}
		if(boat ){
			if(curr->type == BOAT){
				ReachablePlaces[i]= curr->p; 
				i++;
			}
		}
		if(rail){

			if(curr->type == RAIL){	//gota check for drac
				if(player == PLAYER_DRACULA){
					curr=curr->next;
					continue;
				} 	
				int railScore = (round + player) % 4; // as defined in the spec 
					
					if (railScore >= 1) {
						bool visited = false; 
						for (int x = 0; x < i; x++) // check if location has been visited
						{
							if (ReachablePlaces[x] == curr->p) {visited = true; break;}
						}
						if (visited == false) {ReachablePlaces[i] = curr->p; i++;}
						i = getRailLocations(gv, railScore, curr->p, ReachablePlaces, i, 40); // get all rail locations recursively
					}
			}		
		}
		
		curr = curr->next;
	}                               
	*numReturnedLocs = i;
	return ReachablePlaces;
	
}

////////////////////////////////////////////////////////////////////////
// Function returns Player's position in the array
// Input: Round number (MUST be greater than 0)
// Output: returns an array index to start of Dracula's play data in the pastPlays array
// Dracula uses a arithemetic progression formula with a = 32, d = 40
// Lord Godalming uses a arithemetic progression formula with a = 0, d = 40
// Dr Seward uses a arithemetic progression formula with a = 8, d = 40
// Van Helsing uses a arithemetic progression formula with a = 16, d = 40
// Mina Harker uses a arithemetic progression formula with a = 24, d = 40
//! Recommend passing gv->round+1 to the arithmetic progrresion (consult Julian if unsure)
// T_n = a + (n - 1) d
int getPlayer(int round, Player player)
{
	if (round < 1)
		round = 1;
	if (player == PLAYER_DRACULA)
		return (40 * round) - 8;
	if (player == PLAYER_LORD_GODALMING)
		return (40 * round) - 40;
	if (player == PLAYER_DR_SEWARD)
		return (40 * round) - 32;
	if (player == PLAYER_VAN_HELSING)
		return (40 * round) - 24;
	if (player == PLAYER_MINA_HARKER)
		return (40 * round) - 16;
	return -1; // something else went wrong...
}

void initPlayers(GameView gv)
{
	// *Initialise players health's to initial values
	gv->pd->playerHealth[PLAYER_LORD_GODALMING] = GAME_START_HUNTER_LIFE_POINTS;
	gv->pd->playerHealth[PLAYER_DR_SEWARD] = GAME_START_HUNTER_LIFE_POINTS;
	gv->pd->playerHealth[PLAYER_VAN_HELSING] = GAME_START_HUNTER_LIFE_POINTS;
	gv->pd->playerHealth[PLAYER_MINA_HARKER] = GAME_START_HUNTER_LIFE_POINTS;
	gv->pd->playerHealth[PLAYER_DRACULA] = GAME_START_BLOOD_POINTS;

	// *Initialise player locations to 'nowhere'
	for (int i = 0; i < NUM_PLAYERS; i++)
		gv->pd->playerLoc[i] = NOWHERE;

	// *Loop through each round for each player and adjust locations, and health
	for (int round = 0; round <= gv->round; round++)
	{
		char abbrev[3];	  // 2 character string holding the abbreviated location
		abbrev[2] = '\0'; // placeAbbrevToid requires a null character
		for (int player = 0; player < NUM_PLAYERS; player++)
		{
			// * Initialise Variables
			int playerLOC = getPlayer(round + 1, player);
			int rested = false;
			if (playerLOC >= gv->pastPlaysSIZE)
				continue; // outside the array bounds

			// * Update current player's location
			char temp = gv->pastPlays[playerLOC + 1]; // first letter of location abbrev
			abbrev[0] = temp;
			temp = gv->pastPlays[playerLOC + 2]; // second letter of location abbrev
			abbrev[1] = temp;
			PlaceId newID = placeAbbrevToId(abbrev);

			if (player != PLAYER_DRACULA && newID == gv->pd->playerLoc[player])
				rested = true; // if the player's past location is the same as their current, they've rested (only applies to hunters)
			if (gv->pd->playerLoc[player] == ST_JOSEPH_AND_ST_MARY)
				gv->pd->playerHealth[player] = GAME_START_HUNTER_LIFE_POINTS; 
			 
			gv->pd->playerLoc[player] = newID; // Make new id most recent location

			//* Test for double back and update accordingly
			if (player == PLAYER_DRACULA && newID >= DOUBLE_BACK_1 && newID <= DOUBLE_BACK_5) // Dracula made a double back
			{
				newID = getTrail(gv, atoi(&abbrev[1]));
				gv->pd->playerLoc[player] = newID;
			}
			if (player == PLAYER_DRACULA && newID == HIDE)
			{
				newID = getTrail(gv, 1);
				gv->pd->playerLoc[player] = newID;
			}
			if (player == PLAYER_DRACULA && newID == TELEPORT)
			{
				gv->pd->playerLoc[player] = CASTLE_DRACULA; 
			}
			if (player == PLAYER_DRACULA) // add newID to dracula's trail
				addToTrail(gv, newID);
			// * Update curent player's health
			if (player != PLAYER_DRACULA) // player is a hunter
			{
				// Test how many times the Hunter encountered a trap, vampire or dracula (dracula can only be encountered once)
				for (int i = playerLOC + 3; i < playerLOC + 7; i++) // +3 == after location abbrev, +7 == before player move string ends
				{
					if (gv->pastPlays[i] == 'T')
					{
						gv->pd->playerHealth[player] -= LIFE_LOSS_TRAP_ENCOUNTER;						  // life lost for each trap encountered
						gv->ed->locDestroyedTraps[gv->ed->numDestroyedTraps] = gv->pd->playerLoc[player]; //! Unlikely to cause array overflow (may be a bug)
						gv->ed->numDestroyedTraps++;
					}
					if (gv->pastPlays[i] == 'D') // dracula has been encountered (both hunter and dracula lose points)
					{
						gv->pd->playerHealth[player] -= LIFE_LOSS_DRACULA_ENCOUNTER;
						gv->pd->playerHealth[PLAYER_DRACULA] -= LIFE_LOSS_HUNTER_ENCOUNTER;
					}
					if (gv->pastPlays[i] == 'V') // immature vamp encountered (no effect to health simply saves time for later use)
						gv->ed->locDestroyedVamp = gv->pd->playerLoc[player];
				}
				if (gv->pd->playerHealth[player] <= 0) // Hunter is in the hospital (life is fully restored)
				{
					gv->pd->playerHealth[player] = 0; 
					gv->pd->playerLoc[player] = ST_JOSEPH_AND_ST_MARY; 
					gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
					rested = false; 
				}
				if (rested) // if the hunter rested add health points unless its greater than the starting amount of life point (i.e. 9)
				{
					gv->pd->playerHealth[player] += LIFE_GAIN_REST;
					gv->pd->playerHealth[player] = (gv->pd->playerHealth[player] > GAME_START_HUNTER_LIFE_POINTS) ? GAME_START_HUNTER_LIFE_POINTS : gv->pd->playerHealth[player];
				}
			}
			else // player is dracula
			{
				if (placeIdToType(gv->pd->playerLoc[player]) == SEA)
					gv->pd->playerHealth[player] -= LIFE_LOSS_SEA;
				if (gv->pd->playerLoc[player] == CASTLE_DRACULA)
					gv->pd->playerHealth[player] += LIFE_GAIN_CASTLE_DRACULA;
			}
		}
	}
}

// get dracula's location in the trail at a given position
PlaceId getTrail(GameView gv, int position)
{
	if (position > 6)
		position = 6;
	if (position <= 0)
		position = 1;
	for (int i = position - 1; i < TRAIL_SIZE; i++)
	{
		PlaceId place = gv->pd->draculaTrail[i];
		if (placeIsReal(place) || place == CITY_UNKNOWN || place == SEA_UNKNOWN || place == TELEPORT)
			return place;
	}
	return NOWHERE; // Dracula hasn't moved yet
}

// add dracula's most recent location to his trail
// Array order (beginning to end): Most recent Location <----> Oldest Location
void addToTrail(GameView gv, PlaceId location)
{
	for (int i = TRAIL_SIZE - 1; i - 1 >= 0; i--)
		gv->pd->draculaTrail[i] = gv->pd->draculaTrail[i - 1];
	gv->pd->draculaTrail[0] = location;
}

// check if the trap is destroyed if so return true
PlaceId trapIsDestroyed(GameView gv, PlaceId trap)
{
	for (int i = 0; i < gv->round; i++)
	{
		if (trap == gv->ed->locDestroyedTraps[i]) return true;
		char abbrev[3]; abbrev[2] = '\0';
		int dracLoc = getPlayer(i, PLAYER_DRACULA);
		if (dracLoc > gv->pastPlaysSIZE) continue; 
		abbrev[0] = gv->pastPlays[dracLoc+1];
		abbrev[1] = gv->pastPlays[dracLoc+2];
		if (placeAbbrevToId(abbrev) == trap)
		{
			int dracLocTrapMalfunctioned = getPlayer(i+6, PLAYER_DRACULA);
			if (dracLocTrapMalfunctioned > gv->pastPlaysSIZE) continue; 
			if (gv->pastPlays[dracLocTrapMalfunctioned+5] == 'M') return true; 
		}
	}
	return false;
}

// Helper function for GvGetLastMoves and GvGetLastLocations
// copies over PlaceId's in reverse until a limit is reached
// 	- when called by GvGetLastMoves(), placeHistory = moveHistory
// 	- when called by GvGetLastLocations(), placeHistory = locationHistory
PlaceId *GetLastPlaces(GameView gv, Player player, PlaceId *placeHistory, 
						int numPlaces, int *numReturnedPlaces, bool *canFree)
{
	PlaceId *lastPlaces = malloc(*numReturnedPlaces * sizeof(PlaceId));
	int placeCount = 0;
	for (int i = *numReturnedPlaces-1; i >= 0; i--) { // loop through placeHistory array in reverse
		if (numPlaces == 0) break; // stop if the 'numPlaces' place limit has been reached
		lastPlaces[placeCount] = placeHistory[i]; // append to output array
		placeCount++;
		numPlaces--;
	}
	*numReturnedPlaces = placeCount;
	*canFree = false;
	return lastPlaces;
}

// Debugging for Dracula's trail 
void printTrail(GameView gv)
{
	for (int i = 0; i < TRAIL_SIZE; i++)
		printf("%s\n", placeIdToName(gv->pd->draculaTrail[i]));
}

// Given a locations array, add all locations that can be travelled to by rail
// Function uses recursion to get all train connections while the rail score > 1
// Input: gameview, railScore (round +player % 4), source location (must be of type rail)
// pointer to locations array, low == lowest array index without overwriting a location, size == size of the array
// Output: number of added locations (Note: locations array is changed via given pointer)
int getRailLocations(GameView gv, int railScore, PlaceId src, PlaceId *locations, int low, int size) 
{
	int numAddedLocs = low; // set lower bound i.e. where algorithm should start adding locations 
	if (railScore > 1) {
		ConnList railConns = MapGetConnections(gv->GameMap, src); // get all connects of source location
		while (railConns != NULL) { // search through all connections
			if (railConns->type == RAIL) { 
				int visited = false; 
				for (int x = 0; x < size; x++) {
					if (locations[x] == railConns->p) {visited = true; break;}   // check if location has been added already
				}
				if (visited == false) {locations[numAddedLocs] = railConns->p; numAddedLocs++;} // add locations and increment number of added locations
			} 
			railConns = railConns->next; 
		}
		int end = numAddedLocs;
		for (int i = low; i < end; i++) // for each rail location get all its rail connections
		{
			numAddedLocs = getRailLocations(gv, railScore-1, locations[i], locations, numAddedLocs, size); 
		}
	}
	return numAddedLocs; 
}

// gives all locations within a given move radius of a player, ie. all the possible locations they can access within that amount of rounds
PlaceId *moveRadius(GameView gv, Player player, PlaceId current, int radius, int *numReturnedLocations) {

	PlaceId *reachable = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	PlaceId *visited = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	for (int i = 0; i < NUM_REAL_PLACES; i++) visited[i] = -1;    // initialise visited array to all -1's

	int round;
	*numReturnedLocations = 0;
	visited[current] = current; // mark src as visited and add to queue
	Queue Q = newQueue();
	QueueJoin(Q, current);

	while (QueueIsEmpty(Q) == 0) {
	    PlaceId v = QueueLeave(Q);
		reachable[*numReturnedLocations] = v;
		(*numReturnedLocations)++;

		int i = v; // calculate current round by traversing visited array
		round = gv->round + 1;
		while (i != current) {
			i = visited[i];
			round++;
		}

		if (round < gv->round + radius) {
			int numReachableLocs = 0;	// Get all (valid) connections to that city
			PlaceId *connections = GvGetReachable(gv, player, round, v, &numReachableLocs);
			for (int i = 0; i < numReachableLocs; i++) {
				if (visited[connections[i]] == -1) {
					visited[connections[i]] = v;
					QueueJoin(Q, connections[i]);
				}
			}
		}
	}

	free(visited);
	dropQueue(Q);
	return reachable;
}

// uses breadth first search algorithm to find the shortest path between a player and a given destination
PlaceId *GvGetShortestPathTo(GameView gv, Player player, PlaceId dest, int *pathLength)
{
	assert (gv != NULL);

	PlaceId *path = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	PlaceId src = GvGetPlayerLocation(gv, player); // Gets current hunters location
	PlaceId *visited = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	for (int i = 0; i < NUM_REAL_PLACES; i++) visited[i] = -1;    // initialise visited array to all -1's

	int found = 0;
	visited[src] = src;     // mark src as visited and add to queue
	Queue Q = newQueue();
	QueueJoin(Q, src);
	
	while (!found && QueueIsEmpty(Q) == 0) {
	    PlaceId v = QueueLeave(Q);

		int i = v; // calculate current round by traversing visited array
		int round = gv->round;
		while (i != src) {
			i = visited[i]; 
			round++;
		}

	    if (v == dest) {
	        found = 1;
	    } else {    // visit neighbor vertices
			int numReturnedLocs = 0;	// Get all (valid) connections to that city
	        PlaceId *connections = GvGetReachable(gv, player, round, v, &numReturnedLocs);
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

// utility functions for sorting placeId arrays

// compare two placeIds
int placeIdCmp(const void *ptr1, const void *ptr2) {
	PlaceId p1 = *(PlaceId *)ptr1;
	PlaceId p2 = *(PlaceId *)ptr2;
	return p1 - p2;
}
// use qsort and placeIdCmp to sort an array of placeIds
void sortPlaces(PlaceId *places, int numPlaces) {
	qsort(places, (size_t)numPlaces, sizeof(PlaceId), placeIdCmp);
}