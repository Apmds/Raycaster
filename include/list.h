#include <stdbool.h>

#ifndef LIST_H
#define LIST_H

typedef struct list* List;

// Creates a List. printFunc (optional) prints an item from the list)
List ListCreate(void (*printFunc) (void* item));

// Destroys a List
void ListDestroy(List* listp);


// Appends an item to the start of the List, returning whether or not it was successful
bool ListAppendFirst(List list, void* item);

// Appends an item in to the end of the  List, returning whether or not it was successful
bool ListAppendLast(List list, void* item);

// Puts an item in the List, returning whether or not it was successful
bool ListPut(List list, int index, void* item);

// Returns an item from the List
void* ListGet(List list, int index);

// Removes the first item from the List, returning whether or not it was successful
bool ListRemoveFirst(List list, int index);
// Removes the last item from the List, returning whether or not it was successful
bool ListRemoveLast(List list, int index);
// Removes an item from the List, returning whether or not it was successful
bool ListRemove(List list, int index);

// Prints the list in the usual format. printFunc (optional) prints the item correctly)
void ListPrint(List list, bool newline, void (*printFunc) (void* item));

#endif