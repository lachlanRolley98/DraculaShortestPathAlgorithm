// Defines an 'Item' for a Queue ADT
// Designed by team WALJ for Assignment 2 COMP2521 based on implementation by John Shepard 
////////////////////////////////////////////////////////////////////////
// fury-of-dracula-WALJ
// Julian Garratt (z5308427) - H15B - MillenniumForce
// William Dahl (z5317148) - W15B - William-Dahl
// Lachlan Rolley (z5162440) - W15B - LachlanRolley
// Adrian Tran (z5263781) - H15B - ShiniSaba
////////////////////////////////////////////////////////////////////////

#ifndef GLOBAL_H
#define GLOBAL_H

#include "Map.h"

typedef PlaceId Item;

#define ItemCopy(i) (i)
#define ItemEQ(i1, i2) ((i1) == (i2))
#define ItemShow(i) printf ("%d", (i))


#endif