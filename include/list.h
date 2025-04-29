#include <stdbool.h>

#ifndef LIST_H
#define LIST_H

// List type. Pointers stored inside the list are not freed when removing them or destroying the list.
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
bool ListRemoveFirst(List list);

// Removes the last item from the List, returning whether or not it was successful
bool ListRemoveLast(List list);

// Removes an item from the List, returning whether or not it was successful
bool ListRemove(List list, int index);


// Removes the first item from the List, returning it
void* ListPopFirst(List list);

// Removes the last item from the List, returning it
void* ListPopLast(List list);

// Removes an item from the list, returning it
void* ListPop(List list, int index);


// List Looping functions

// Moves the pointer to the start
void ListMoveToStart(List list);

// Moves the pointer to the next element (returns false if at the end)
bool ListMoveToNext(List list);

// Moves the pointer to the end of the list
void ListMoveToEnd(List list);

// Returns the current item from the list
void* ListGetCurrent(List list);

// Returns whether or not the list has a next element
bool ListHasNext(List list);


// Prints the list in the usual format. printFunc (optional) prints the item correctly)
void ListPrint(List list, bool newline, void (*printFunc) (void* item));

#endif