#include <stdlib.h>
#include <assert.h>
#include "list.h"

typedef struct listnode* ListNode;

struct listnode {
    void* value;
    ListNode nextNode;
};

struct list {
    ListNode firstNode;
    ListNode currentNode;       // Node somewhere in the list where we can start operations in
    int size;
};


// Creates a List
List ListCreate() {
    List list = malloc(sizeof(struct list));
    assert(list != NULL);
    
    list->size = 0;
}

// Destroys a List
void ListDestroy(List* listp) {
    assert(listp != NULL);
    assert(*listp != NULL);

    List list = *listp;
    ListNode node = list->firstNode;
    ListNode lastNode = list->firstNode;

    // Free all the list nodes
    while (node != NULL) {
        lastNode = node;
        node = node->nextNode;

        free(lastNode);
    }

    free(list);
    *listp = NULL;
}


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