////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
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
#include <string.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"

void HunterMove(PlaceId ID);
void printGameState(HunterView hv);

////////////////////////////////////////////////////////////////////////
void decideHunterMove(HunterView hv)
{
	//our initializers

	Round round = HvGetRound(hv);
	//int score = HvGetScore(hv);
	//PlaceId vamp_loc = HvGetVampireLocation(hv);
	Round round_drac_find = round;
	PlaceId drac_loc = NOWHERE;
	//if (round == 1 && HvGetPlayer(hv) == 0) printf("STOP");
	if (round >= 1) {
		drac_loc = HvGetLastKnownDraculaLocation(hv, &round_drac_find);
	}
	//printf("\n%s\n", placeIdToName(drac_loc));

	Player player = HvGetPlayer(hv);
	int health = HvGetHealth(hv, player);
	PlaceId curr_loc = HvGetPlayerLocation(hv, player);

	// printGameState(hv); // For debugging 

	// this is our opening hand, basically we have drawn on a map the best routes to cover a large span of places
	// on round 6, if we havent found draci boy (which prob havent cos peoples drac view will be run from hunters however good chance of finding line)
	// we finish in strong spots on a trainline and research 
	// if at any point drac is found or a location in his line we break out of this set code
	if (!placeIsReal(drac_loc) && round < 7) {
		if (round == 0) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(EDINBURGH);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(BRUSSELS);
			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(NANTES);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(ATHENS);
			}
		}
		if (round == 1) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(MANCHESTER);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(AMSTERDAM);
			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(BORDEAUX);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(VALONA);
			}
		}
		if (round == 2) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(LIVERPOOL);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(COLOGNE);
			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(SARAGOSSA);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(SALONICA);
			}
		}
		if (round == 3) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(SWANSEA);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(HAMBURG);

			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(SANTANDER);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(SOFIA);

			}
		}
		if (round == 4) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(LONDON);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(BERLIN);
			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(MADRID);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(VARNA);
			}
		}
		if (round == 5) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(PLYMOUTH);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(LEIPZIG);
			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(LISBON);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(CONSTANTA);
			}
		}
		//if we get here, it means havent found path so we just research
		if (round == 6) {
			if (player == PLAYER_LORD_GODALMING) {
				HunterMove(PLYMOUTH);
			}
			if (player == PLAYER_DR_SEWARD) {
				HunterMove(LEIPZIG);
			}
			if (player == PLAYER_VAN_HELSING) {
				HunterMove(LISBON);
			}
			if (player == PLAYER_MINA_HARKER) {
				HunterMove(CONSTANTA);
			}
		}
	}

	if (round <= 6 && !placeIsReal(drac_loc)) return; // above if statements should have been invoked and move submitted

	if ((HvGetRound(hv) - round_drac_find) >= 9 || (health <= 2 && curr_loc != ST_JOSEPH_AND_ST_MARY)) // better to rest and find dracs location also rest if health is low
	{
		HunterMove(curr_loc); // submit rest move and exit  
		return;
	}

	int numReturnedLocs = -1;
	int searchRadius = HvGetRound(hv) - round_drac_find; // narrow down the sarch radius (current round minus round of where Hunter's suspect Dracula last was)
	PlaceId *pathRadius = HvplayermoveRadius(hv, PLAYER_DRACULA, drac_loc, searchRadius, &numReturnedLocs); // start investigation
	int suspectLocation = pathRadius[rand()%numReturnedLocs]; // set as initial location before loop
	for (int i = 0; i < numReturnedLocs; i++)
	{
		if (((i+1) % (player+2)) == 0 && pathRadius[i] != curr_loc) {
			suspectLocation = pathRadius[i]; break;
		} // first location not visited
		// mod is used (player+2 as cannot modulo 0 and everything mod 1 is 0) so that each hunter gets a distinct location to search
	}
	if ((curr_loc == pathRadius[0] && numReturnedLocs == 1) || suspectLocation == curr_loc)
	{ // 1st condition conveys the fact that if a hunter is at the only suspect location go to that location
	  // 2nd condition conveys that if the suspect location is the current location don't run path find just move there straight away
		HunterMove(pathRadius[0]);
	}
	else
	{ // Go to another suspected location 
		int pathLength = -1;
		PlaceId *path = HvGetShortestPathTo(hv, player, suspectLocation, &pathLength); // get shortest path
		if (isConnected(hv, curr_loc, path[0])) {
			HunterMove(path[0]);
		} // move to the first place in the shortest path if connected
		free(path);
	}
	free(pathRadius);
}

void HunterMove(PlaceId ID) {
	Message message = "Your moves are weak Draci boy";
	registerBestPlay(placeIdToAbbrev(ID), message);
}

void printGameState(HunterView hv)
{
	printf("\n");
	// Print hunter's moves
	for (Player player = PLAYER_LORD_GODALMING; player <= PLAYER_DRACULA; player++)
	{
		int numReturnedLocs = 0;
		PlaceId *pastPlaces = HvlocationHistory(hv, player, &numReturnedLocs);
		switch (player)
		{
		case 0:
			printf("0: Lord Goldalming[%d] play history: ", HvGetHealth(hv, player));
			break;
		case 1:
			printf("1: Dr Seward[%d] play history: ", HvGetHealth(hv, player));
			break;
		case 2:
			printf("2: Van Helsing[%d] play history: ", HvGetHealth(hv, player));
			break;
		case 3:
			printf("3: Mina Harker[%d] play history: ", HvGetHealth(hv, player));
			break;
		default:
			printf("4: Dracula[%d] play history: ", HvGetHealth(hv, player));
			break;
		}
		for (int i = 0; i < numReturnedLocs; i++)
		{
			printf("%s -> ", placeIdToName(pastPlaces[i]));
		}
		printf("\n");
		free(pastPlaces);
	}

	printf("\nGame Score = %d Round = %d\n", HvGetScore(hv), HvGetRound(hv));

}