////////////////////////////////////////////////////////////////////////
// Runs a player's game turn ...
//
// Can  produce  either a hunter player or a Dracula player depending on
// the setting of the I_AM_DRACULA #define
//
// This  is  a  dummy  version of the real player.c used when you submit
// your AIs. It is provided so that you can test whether  your  code  is
// likely to compile ...
//
// Note that this is used to drive both hunter and Dracula AIs. It first
// creates an appropriate view, and then invokes the relevant decideMove
// function,  which  should  use the registerBestPlay() function to send
// the move back.
//
// The real player.c applies a timeout, and will halt your  AI  after  a
// fixed  amount of time if it doesn 't finish first. The last move that
// your AI registers (using the registerBestPlay() function) will be the
// one used by the game engine. This version of player.c won't stop your
// decideMove function if it goes into an infinite loop. Sort  that  out
// before you submit.
//
// Based on the program by David Collien, written in 2012
//
// 2017-12-04	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.3	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#ifdef I_AM_DRACULA
# include "dracula.h"
# include "DraculaView.h"
#else
# include "hunter.h"
# include "HunterView.h"
#endif

// Moves given by registerBestPlay are this long (including terminator)
#define MOVE_SIZE 3

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

// A pseudo-generic interface, which defines
// - a type `View',
// - functions `ViewNew', `decideMove', `ViewFree',
// - a trail `xtrail', and a message buffer `xmsgs'.
#ifdef I_AM_DRACULA

typedef DraculaView View;

# define ViewNew DvNew
# define decideMove decideDraculaMove
# define ViewFree DvFree

// used for testing
# define xMsgs { "", "", "", "" }
# define xPastPlays "GED.... SBU.... HNA.... MAT.... DCD.V.. GNS.... SFR.... HCF.... MIO.... DKLT... GHA.... SMU.... HGE.... MBS.... DD1T... GVI.... SZA.... HMI.... MCN.... DHIT... GBD.... SSZ.... HVE.... MBD.... DBET... GKLTTT. SGA.... HBD.... MVI.... DSJT... GSZ.... SBC.... HKL.... MBD.... DVAT.V. GGA.... SGA.... HCD.... MVI.... DIO.... GBC.... SBC.... HGA.... MBD.... DTS.... GSZ.... SGA.... HCD.... MVI.... DMS.... GGA.... SBC.... HKL.... MBD.... DAO..M. GGA.... SBC.... HKL.... MBD.... DEC..M. GBC.... SSO.... HBE.... MBE.... DNS..M. GSZ.... SVA.... HSO.... MSJ.... DD3.... GZA.... SSJ.... HSJ.... MBE.... DBB.... GZA.... SSJ.... HSJ.... MBE.... DSNT... GMU.... SZA.... HVA.... MSA.... DMAT... GMI.... SMU.... HIO.... MIO.... DCAT... GGO.... SMI.... HTS.... MTS.... DAO.... GGO.... SMI.... HTS.... MTS.... DMS.... GGO.... SGE.... HGO.... MMS.... DTS.... GMR.... SCF.... HGE.... MAO.... DIO..M. GBO.... SNA.... HCF.... MEC.... DBS..M. GSN.... SBO.... HBO.... MAO.... DCNT.M. GSN.... SBO.... HBO.... MAO.... DGAT... GBB.... SBA.... HLS.... MIR.... DCDT... GBO.... SMS.... HAO.... MAO.... DKL.V.. GMR.... STS.... HNS.... MMS.... DBCT... GGO.... SIO.... HHA.... MMR.... DD2T... GVE.... SBS.... HVI.... MGO.... DHIT.M. GBD.... SCN.... HBD.... MVE.... DBET.M. GKLTTV. SGA.... HKL.... MBD.... DSOT.M. GSZ.... SBCT... HGA.... MCN.... DVAT... GBET... SSOT... HBE.... MBS.... DIO.... GVR.... SVAT... HSJ.... MIO.... DTS.... GBS.... SSO.... HBE.... MSA.... DMS.... GIO.... SVA.... HSJ.... MVR.... DAO.... GVA.... SSO.... HSO.... MSO.... DEC.... GIO.... SVA.... HSJ.... MSA.... DNS.... GVA.... SSO.... HBE.... MVR.... DD3.... GIO.... SVA.... HSJ.... MSA.... DBB.... GVA.... SSO.... HSO.... MSO.... DNAT... GVA.... SSO.... HSO.... MSO.... DCFT... GIO.... SSA.... HSA.... MSA.... DGET... GTS.... SIO.... HIO.... MIO.... DSTT... GMS.... STS.... HTS.... MTS.... DMUT... GMS.... STS.... HTS.... MTS.... DVIT... GAL.... SGO.... HGO.... MMS.... DBDT.M. GMA.... SMR.... HMR.... MAO.... DKLT.M. GSN.... SBO.... HBO.... MEC.... DCDT.M. GSR.... SMR.... HCF.... MNS.... DHIT.M. GMR.... SGO.... HGE.... MHA.... DGAT.M. GGO.... SVE.... HMI.... MVIT... DCN.V.. GVE.... SBDT... HMU.... MBD.... DBS.... GBD.... SGAT... HVI.... MKLT... DIO.... GKL.... SCNV... HBD.... MBC.... DTS..M. GBC.... SVR.... HBE.... MBE.... DMS..M. GCN.... SBS.... HGA.... MBC.... DAO.... GBS.... SVR.... HKL.... MBE.... DEC.... GCN.... SBS.... HGA.... MBC.... DNS.... GBS.... SVR.... HKL.... MBE.... DD3.... GCN.... SBS.... HGA.... MBC.... DBB.... GCN.... SBS.... HGA.... MBC.... DNAT... GBS.... SIO.... HCN.... MBE.... DCFT... GIO.... STS.... HBS.... MSA.... DPAT... GTS.... SGO.... HIO.... MIO.... DST.V.. GTS.... SGO.... HIO.... MIO.... DMUT... GTS.... SMR.... HTS.... MTS.... DVIT... GMS.... SCFT... HGO.... MGO.... DBDT.M. GBA.... SPAT... HGE.... MMR.... DKLT... GBO.... SNA.... HPA.... MGE.... DCDT... GMR.... SPA.... HSTV... MMI.... DHIT... GGO.... SST.... HLI.... MMUT... DGAT... GVE.... SLI.... HMU.... MVIT... DBCT... GBDT... SMU.... HZA.... MBD.... DSOT... GKLT... SZA.... HSZ.... MVI.... DVAT... GSZ.... SSZ.... HCN.... MBD.... DIO..M. GKL.... SGAT... HGA.... MKL.... DTS..M. GCD.... SCN.... HCD.... MBCT... DMS.... GGA.... SBC.... HGA.... MCN.... DAO.... GBC.... SKL.... HBC.... MBE.... DEC..M. GSO.... SBE.... HGA.... MBC.... DNS..M. GBE.... SKL.... HBC.... MCN.... DD3.... GBE.... SKL.... HBC.... MCN.... DBB.... GKL.... SBE.... HBE.... MBS.... DNAT... GBD.... SSA.... HSA.... MIO.... DCFT... GVE.... SIO.... HIO.... MTS.... DGET... GVE.... SIO.... HIO.... MTS.... DSTT... GMI.... STS.... HTS.... MMS...."
#else

typedef HunterView View;

# define ViewNew HvNew
# define decideMove decideHunterMove
# define ViewFree HvFree

// used for testing
# define xPastPlays "GED.... SBU.... HNA.... MAT.... DCD.V.."
# define xMsgs { "", "", "" }

#endif

int main(void) // irrespecive of whether it's a hunter or dracula, calls appropriate functions to decide a move and prints it
{
	char *pastPlays = xPastPlays;
					  
	Message msgs[] = xMsgs;

	View state = ViewNew(pastPlays, msgs);
	decideMove(state);
	ViewFree(state);

	printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
	return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay(char *play, Message message)
{
	strncpy(latestPlay, play, MOVE_SIZE - 1);
	latestPlay[MOVE_SIZE - 1] = '\0';

	strncpy(latestMessage, message, MESSAGE_SIZE - 1);
	latestMessage[MESSAGE_SIZE - 1] = '\0';
}

