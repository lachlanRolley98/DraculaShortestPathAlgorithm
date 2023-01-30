// Queue.h ... interface to Queue ADT
// Written by John Shepherd, March 2013 adapted for Assignment 2 for team WALJ 
////////////////////////////////////////////////////////////////////////
// fury-of-dracula-WALJ
// Julian Garratt (z5308427) - H15B - MillenniumForce
// William Dahl (z5317148) - W15B - William-Dahl
// Lachlan Rolley (z5162440) - W15B - LachlanRolley
// Adrian Tran (z5263781) - H15B - ShiniSaba
////////////////////////////////////////////////////////////////////////

#ifndef QUEUE_H
#define QUEUE_H

#include "Global.h"

typedef struct QueueRep *Queue;

Queue newQueue (void);			// create new empty queue
void dropQueue (Queue);			// free memory used by queue
void showQueue (Queue);			// display as 3 > 5 > 4 > ...
void QueueJoin (Queue, Item);	// add item on queue
Item QueueLeave (Queue);		// remove item from queue
int QueueIsEmpty (Queue);		// check for no items

#endif