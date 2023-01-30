#ifndef FOD__HEAP_H_
#define FOD__HEAP_H_

#include "Places.h"

typedef struct weightedLocation *Item; // one item is a single weight of a location
typedef struct HeapRep *Heap;
typedef Heap PQueue; 

// Create new heap of size N
Heap newHeap(int N); 

// Add a new location, weight tuple to the heap
void joinHeap(PQueue pq, int weight, PlaceId location);

// Remove an item from the heap 
// Returned value is in the form of a placeID
int leaveHeap(PQueue pq);

// Return 1 if heap is empty
// Return 0 if heap is not empty
int isEmpty(PQueue h);

// Print priority queue
// Solely for debugging
void printPQueue(PQueue pq); 

// Free a given priority queue 
void freePQueue(PQueue pq); 

#endif