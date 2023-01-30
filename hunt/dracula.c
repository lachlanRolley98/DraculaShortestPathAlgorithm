
////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
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

#include <stdio.h>

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "Heap.h"

////////////////////////////////////////////////////////////////////////

// Function returns the location a double back is referring to
// If location exists return the PlaceId else return NOWEHRE
PlaceId doubleBackLocation(DraculaView dv, int doubleBackSize);

// prints the past places string for each player in the game, useful for debugging
void printGameState(DraculaView dv);

////////////////////////////////////////////////////////////////////////
void decideDraculaMove(DraculaView dv)
{
	if (DvGetRound(dv) != 0)
	{
		// Get all moves connected to Dracula (DvGetValidMoves however must cover Hide/double back into valid places)
		int numReturnedLocs = 0;
		PlaceId *moves = DvGetValidMoves(dv, &numReturnedLocs);
		// printGameState(dv); // for debugging
		if (numReturnedLocs == 0)
		{
			registerBestPlay("TP", "yikes");
			free(moves);
			return;
		}

		int *weightedLocs = mapWeightings(dv);

		// Get all weights associated to that location
		PQueue pq = newHeap(numReturnedLocs + 1);
		for (int i = 0; i < numReturnedLocs; i++)
		{
			// check if location is a double back if so give it the weight of the location it refers to
			if (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_5)
			{
				PlaceId dbLocation = doubleBackLocation(dv, (moves[i]) - DOUBLE_BACK_1);
				joinHeap(pq, weightedLocs[dbLocation] - 5, moves[i]); // add location/move + weight of location of double back to heap
			}
			else if (moves[i] == HIDE)
			{
				PlaceId hideLocation = doubleBackLocation(dv, 0);
				joinHeap(pq, weightedLocs[hideLocation] - 5, moves[i]); // add location + weight to heap
			}
			else if (moves[i] == TELEPORT)
				continue;
			else
			{
				joinHeap(pq, weightedLocs[moves[i]], moves[i]);
			} // add location + weight to heap
		}

		printPQueue(pq);

		// Get best location (i.e. root of the heap)
		PlaceId bestLocation;
		if (isEmpty(pq))
		{
			bestLocation = TELEPORT;
		}
		else
		{
			bestLocation = leaveHeap(pq);
		}

		bool no_hunters = true;
		// If dracula's health below X blood points and castle dracula is clear of 2 hunters within a 2 round radius
		if (DvGetHealth(dv, PLAYER_DRACULA) <= 30 && playersNearCD(dv, 3) < 2)
		{
			//	Call DvGetPath to castle dracula // this will be the one in game view
			//	this will return an array of place id locations and a size of this array
			int castle_move_length;
			PlaceId *drac_options;
			if (DvGetHealth(dv, PLAYER_DRACULA) < 10)
			{ // if health is really low prioritise safest path
				drac_options = DvGetSafestPathTo(dv, PLAYER_DRACULA, CASTLE_DRACULA, &castle_move_length);
			}
			else
			{
				drac_options = DvGetShortestPathTo(dv, PLAYER_DRACULA, CASTLE_DRACULA, &castle_move_length);
			}

			for (int x = 0; x < PLAYER_DRACULA && no_hunters; x++)
			{ // this will run through all players
				int numReturnedLocs;
				PlaceId *reachable_locs = DvWhereCanTheyGo(dv, x, &numReturnedLocs);
				// at this point we got an array of all avaliable locations for this specific hunter
				for (int a = 0; a < numReturnedLocs; a++)
				{
					if (reachable_locs[a] == drac_options[0])
					{
						no_hunters = false;
						break;
					}
				}
				free(reachable_locs);
			}
			if (DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA)
			{ // Dracula is currently is at Dracula
				for (int i = 0; i < numReturnedLocs; i++)
				{
					if (moves[i] == HIDE || moves[i] == DOUBLE_BACK_1)
					{ // If dracula is at CD and he is still low on BP try to do a DB or HI
						registerBestPlay(placeIdToAbbrev(moves[i]), "Insert witty comment about evading hunters");
						freePQueue(pq);
						free(moves);
						free(weightedLocs);
						free(drac_options);

						printf("Best Location: %s\n", placeIdToName(bestLocation));
						printGameState(dv);
						return;
					}
				}
			}
			else if (no_hunters)
			{ // IF no hunters are around CD go on a path to CD
				bool isValid = false;
				for (int i = 0; i < numReturnedLocs; i++)
				{ // Check if the first option in the shortest path is a valid move
					if (moves[i] == drac_options[0])
					{
						isValid = true;
						break;
					}
				}
				if (isValid)
				{ // if valid make that move
					registerBestPlay(placeIdToAbbrev(drac_options[0]), "Insert witty comment about evading hunters");

					freePQueue(pq);
					free(moves);
					free(weightedLocs);
					free(drac_options);

					printf("Best Location: %s\n", placeIdToName(bestLocation)); // used for debugging
					printGameState(dv);
					return;
				}
			}
		}

		// Free pq and weightedLocs array
		freePQueue(pq);
		free(moves);
		free(weightedLocs);

		printf("Best Location: %s\n", placeIdToName(bestLocation));
		printGameState(dv);

		// Make/submit move (this will occur if Dracula isn't on a path to CD or if CD is surrounded by hunters)
		registerBestPlay(placeIdToAbbrev(bestLocation), "Insert witty comment about evading hunters");
		return;
	}

	// Below occurs only in round 1
	int numSafeLocs;
	PlaceId *safeLocs;
	for (int i = 5; i > 0; i--)
	{ // get all safe locations
		safeLocs = unreachables(dv, i, &numSafeLocs);
		if (numSafeLocs > 0)
			break;
	}

	PlaceId bestLocation;
	int mostConnections = 0, connections;
	for (int i = 0; i < numSafeLocs; i++)
	{ // choose the safest location that also has the greatest number of connections
		connections = locationConnections(dv, safeLocs[i]);
		if (connections > mostConnections)
		{
			mostConnections = connections;
			bestLocation = safeLocs[i];
		}
	}

	registerBestPlay(placeIdToAbbrev(bestLocation), "Mwahahahaha");
	free(safeLocs);
	return;
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

// prints the past places string for each player in the game, useful for debugging
void printGameState(DraculaView dv)
{
	printf("\n");
	// Print hunter's moves
	for (Player player = PLAYER_LORD_GODALMING; player <= PLAYER_DRACULA; player++)
	{
		int numReturnedLocs = 0;
		PlaceId *pastPlaces = locationHistory(dv, player, &numReturnedLocs);
		switch (player)
		{
		case 0:
			printf("Lord Goldalming[%d] play history: ", DvGetHealth(dv, player));
			break;
		case 1:
			printf("Dr Seward[%d] play history: ", DvGetHealth(dv, player));
			break;
		case 2:
			printf("Van Helsing[%d] play history: ", DvGetHealth(dv, player));
			break;
		case 3:
			printf("Mina Harker[%d] play history: ", DvGetHealth(dv, player));
			break;
		default:
			printf("Dracula[%d] play history: ", DvGetHealth(dv, player));
			break;
		}
		for (int i = 0; i < numReturnedLocs; i++)
		{
			printf("%s -> ", placeIdToName(pastPlaces[i]));
		}
		printf("\n\n");
		free(pastPlaces);
	}

	printf("\nGame Score = %d Round = %d\n", DvGetScore(dv), DvGetRound(dv));
}