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
bool ListAppendFirst(List list, void* item) {
    if (list == NULL) {
        return false;
    }

    ListNode node = malloc(sizeof(struct listnode));
    if (node == NULL) {
        return false;
    }

    node->value = item;
    node->nextNode = list->firstNode;
    
    // Change the current node if its pointing to the first node
    if (list->currentNode == list->firstNode) {
        list->currentNode = node;
    }
    list->firstNode = node;
    list->size++;

    return true;
}

// Appends an item in to the end of the  List, returning whether or not it was successful
bool ListAppendLast(List list, void* item) {
    if (list == NULL) {
        return false;
    }

    // Loop until last index
    while (list->currentNode->nextNode != NULL) {
        list->currentNode=list->currentNode->nextNode;
    }

    // Creating new node
    ListNode node = malloc(sizeof(struct listnode));
    if (node == NULL) {
        return false;
    }
    node->value = item;

    // Put node at the end
    list->currentNode->nextNode = node;
    list->size++;

    return true;
}

// Puts an item in the List, returning whether or not it was successful
bool ListPut(List list, int index, void* item) {
    if (list == NULL) {
        return false;
    }

    // Index at the lists edges
    if (index == 0) {
        return ListAppendFirst(list, item);
    }
    if (index == list->size) {
        return ListAppendLast(list, item);
    }

    // Invalid index
    if (index > list->size) {
        return false;
    }

    // Loop until right index
    ListNode lastNode = NULL;
    list->currentNode = list->firstNode;
    int i = 0;
    for (i = 0; i < index && list->currentNode != NULL; i++) {
        lastNode = list->currentNode;
        list->currentNode=list->currentNode->nextNode;
    }

    // Didn't reach the desired index
    if (list->currentNode == NULL && i < index) {
        return false;
    }    

    // Creating new node
    ListNode node = malloc(sizeof(struct listnode));
    if (node == NULL) {
        return false;
    }
    node->value = item;
    
    // node points to currentNode and lastNode points to node
    node->nextNode = list->currentNode;
    lastNode->nextNode = node;
    list->size++;

    return true;
}

// Returns an item from the List
void* ListGet(List list, int index);

// Removes the first item from the List, returning whether or not it was successful
bool ListRemoveFirst(List list, int index);
// Removes the last item from the List, returning whether or not it was successful
bool ListRemoveLast(List list, int index);
// Removes an item from the List, returning whether or not it was successful
bool ListRemove(List list, int index);