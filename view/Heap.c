/////////////////////////////////////////////////////////////////////////////
/*
• Written by team William-Adrian-Lachlan-Julian in July 2020
• Code adapted from John Shepard's Heaps and Priority Queues Slides 5/7/2020
• Implements a heap ADT for the Fury of Dracula Assignment COMP2521 2020 T2
• Heap structed by location weights where the highest weight is given the
  greatest priority
*/
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "Places.h"
#include "Heap.h"

typedef struct HeapRep
{
   Item *items; // array of location weights and its corresponding location
   int nitems;  // #items in array
   int nslots;  // #elements in array
} HeapRep;

typedef struct weightedLocation
{
   PlaceId location;
   int weight;
} weightedLocation;

static void HeapInsert(Heap h, Item it);
static Item HeapDelete(Heap h);
static void fixUp(Item a[], int i);
static void swap(Item a[], int i, int j);
static void fixDown(Item a[], int i, int N);
static int greater(Item a, Item b);

Heap newHeap(int N)
{
   Heap new = malloc(sizeof(HeapRep));
   Item *a = malloc((N + 1) * sizeof(Item));
   assert(new != NULL &&a != NULL);
   new->items = a;  // no initialisation needed
   new->nitems = 0; // counter and index
   new->nslots = N; // index range 1..N
   return new;
}

void joinHeap(PQueue pq, int weight, PlaceId location)
{
   Item new = malloc(sizeof(weightedLocation));
   assert(new != NULL);
   assert(weight >= 0);
   assert(placeIsReal(location) != false);
   new->location = location;
   new->weight = weight;
   HeapInsert(pq, new);
}

int leaveHeap(PQueue pq)
{
   Item out = HeapDelete(pq);
   int placeID = out->location;
   free(out);
   return placeID;
}

void printPQueue(PQueue pq)
{
   for (int i = 1; i < pq->nitems; i++)
   {
      const char *location = placeIdToName(pq->items[i]->location);
      printf("{In[%d] %s:%d} \n", i, location, pq->items[i]->weight);
   }
}

int is_empty(Heap h)
{
   // if empty return 1
   return (h->nitems == 0);
}

void freePQueue(PQueue pq)
{
   for (int i = 1; i <= pq->nitems; i++)
   {
      Item current = pq->items[i];
      free(current);
   }
   free(pq->items);
   free(pq); 
}

////////////////////////////////////////////////////////////////////////
//                            HELPER                                  //
//                           FUNCTIONS                                //
////////////////////////////////////////////////////////////////////////

static void HeapInsert(Heap h, Item it)
{
   // is there space in the array?
   assert(h->nitems < h->nslots);
   h->nitems++;
   // add new item at end of array
   h->items[h->nitems] = it;
   // move new item to its correct place
   fixUp(h->items, h->nitems);
}

static Item HeapDelete(Heap h)
{
   Item top = h->items[1];
   // overwrite first by last
   h->items[1] = h->items[h->nitems];
   h->nitems--;
   // move new root to correct position
   fixDown(h->items, 1, h->nitems);
   return top;
}

// force value at a[i] into correct position
static void fixUp(Item a[], int i)
{
   while (i > 1 && greater(a[i / 2], a[i]))
   {                     // greater function takes parent of node i, and i
      swap(a, i, i / 2); // swap the parent of i and node i
      i = i / 2;         // integer division
   }
}

static void swap(Item a[], int i, int j)
{
   Item tmp = a[i];
   a[i] = a[j];
   a[j] = tmp;
}

static void fixDown(Item a[], int i, int N)
{
   while (2 * i <= N)
   {
      // compute address of left child
      int j = 2 * i;
      // choose larger of two children
      if (j < N && greater(a[j], a[j + 1]))
         j++; //  if j is greater than its left child j becomes left child
      if (!greater(a[i], a[j]))
         break;
      swap(a, i, j);
      // move one level down the heap
      i = j;
   }
}

static int greater(Item a, Item b)
{
   // if b is greater than a return 1
   return (a->weight <= b->weight);
}