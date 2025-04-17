#include <stdbool.h>

#ifndef LIST_H
#define LIST_H

typedef struct list* List;

// Creates a List
List ListCreate();

// Destroys a List
void ListDestroy(List* list);


// Appends an item to the start of the List, returning whether or not it was successful
bool ListAppendFirst(List map, void* item);

// Appends an item in to the end of the  List, returning whether or not it was successful
bool ListAppendLast(List map, void* item);

// Puts an item in the List, returning whether or not it was successful
bool ListPut(List map, int index, void* item);

// Returns an item from the List
void* ListGet(List map, int index);

// Removes the first item from the List, returning whether or not it was successful
bool ListRemoveFirst(List map, int index);
// Removes the last item from the List, returning whether or not it was successful
bool ListRemoveLast(List map, int index);
// Removes an item from the List, returning whether or not it was successful
bool ListRemove(List map, int index);

#endif