////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testDraculaView.c: test the DraculaView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-02	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
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

#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for basic functions, "
			   "just before Dracula's first move\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "..."
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 0);
		assert(DvGetScore(dv) == GAME_START_SCORE);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == ZURICH);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD.. SAO.... HCD.... MAO....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahah",
			"Aha!", "", "", ""
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 1);
		assert(DvGetScore(dv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula leaving minions 1\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 4);
		assert(DvGetVampireLocation(dv) == EDINBURGH);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == LONDON);
		assert(traps[1] == LONDON);
		assert(traps[2] == MANCHESTER);
		free(traps);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves 0 (Dracula hasn't had a turn yet\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		assert(numMoves == 0);
		assert(moves == NULL);
		DvFree(dv);

		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves 1 (Standard Test)\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		// printf("%d\n", numMoves);
		// for (int i = 0; i < numMoves; i++)
		// 	printf("%s\n", placeIdToName(moves[i]));
		assert(numMoves == 4);
		sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
		assert(moves[1] == KLAUSENBURG);
		assert(moves[2] == HIDE);
		assert(moves[3] == DOUBLE_BACK_1);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
		
		printf("Test for Dracula's valid moves 2 (Checking if Dracula won't go back to a location in his trail unless by double back)\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE.... DKLT... "
			"GGE.... SGE.... HGE.... MGE.... DBDT... "
			"GGE.... SGE.... HGE.... MGE.... DVIT... "
			"GGE.... SGE.... HGE.... MGE.... DZAT... "
			"GGE.... SGE.... HGE.... MGE.... DSZT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		// printf("%d\n", numMoves);
		// for (int i = 0; i < numMoves; i++)
		// 	printf("%s\n", placeIdToName(moves[i]));
		
		assert(numMoves == 6);
		sortPlaces(moves, numMoves);
		assert(moves[0] == BELGRADE);
		assert(moves[1] == HIDE);
		assert(moves[2] == DOUBLE_BACK_1);
		assert(moves[3] == DOUBLE_BACK_2);
		assert(moves[4] == DOUBLE_BACK_4);
		assert(moves[5] == DOUBLE_BACK_5);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
		
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanIGo\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == CONSTANTA);
		assert(locs[2] == GALATZ);
		assert(locs[3] == SOFIA);
		free(locs);
		
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanIGoByType\n\tVarna by boat\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE.... DGAT... "
			"GGE.... SGE.... HGE.... MGE.... DCNT... "
			"GGE.... SGE.... HGE.... MGE.... DVRT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, false, true, &numLocs);
		assert(numLocs == 1);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BLACK_SEA);
		free(locs);

		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("\tVarna by road\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE.... DGAT... "
			"GGE.... SGE.... HGE.... MGE.... DCNT... "
			"GGE.... SGE.... HGE.... MGE.... DVRT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, true, false, &numLocs);
		assert(numLocs == 1);
		sortPlaces(locs, numLocs);
		assert(locs[0] == SOFIA);
		free(locs);
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("\tCagliari by boat\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCGT... ";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, false, true, &numLocs);
		assert(numLocs == 2);
		sortPlaces(locs, numLocs);
		assert(locs[0] == MEDITERRANEAN_SEA);
		assert(locs[1] == TYRRHENIAN_SEA);
		free(locs);
		
		printf("Tests passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanTheyGo - From Vienna as Lord Godalming R2\n");

		char *trail =
			"GVI.... SGE.... HGE.... MGE.... DCGT... ";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_LORD_GODALMING, &numLocs);

		//printf("\n%d\n", numLocs);
		//for (int i = 0; i < numLocs; i++)
		//  	printf("\n%s\n", placeIdToName(locs[i]));
		assert(numLocs == 5);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BUDAPEST);
		assert(locs[1] == MUNICH);
		assert(locs[2] == PRAGUE);
		assert(locs[3] == VENICE);
		assert(locs[4] == ZAGREB);

		free(locs);
		DvFree(dv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanTheyGoByType - Ionian Sea boat connections "
			       "(Lord Godalming, Round 1)\n");

		char *trail =
			"GIO.... SGE.... HGE.... MGE.... ";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		//printf("\n%d\n", numLocs);
		//for (int i = 0; i < numLocs; i++)
		//  	printf("\n%s\n", placeIdToName(locs[i]));
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGoByType(dv, PLAYER_LORD_GODALMING, false, false,
												true, &numLocs);
		assert(numLocs == 6);
		sortPlaces(locs, numLocs);
		assert(locs[0] == ADRIATIC_SEA);
		assert(locs[1] == ATHENS);
		assert(locs[2] == BLACK_SEA);
		assert(locs[3] == SALONICA);
		assert(locs[4] == TYRRHENIAN_SEA);
		assert(locs[5] == VALONA);
		free(locs);
		DvFree(dv);
		printf("Test passed!\n");
	}

	return EXIT_SUCCESS;
}
