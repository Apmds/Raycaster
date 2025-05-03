#include <stdlib.h>
#include <stdio.h>
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
    void (*printFunc) (void* item);
    int size;
};


// Creates a List. printFunc (optional) prints an item from the list)
List ListCreate(void (*printFunc) (void* item)) {
    List list = malloc(sizeof(struct list));
    assert(list != NULL);
    
    list->firstNode = NULL;
    list->currentNode = NULL;
    list->printFunc = printFunc;
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
    assert(list != NULL);

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
    assert(list != NULL);
    
    if (list->size == 0) {
        return ListAppendFirst(list, item);
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
    node->nextNode = NULL;

    // Put node at the end
    list->currentNode->nextNode = node;
    list->size++;

    return true;
}

// Puts an item in the List, returning whether or not it was successful
bool ListPut(List list, int index, void* item) {
    assert(list != NULL);

    // Invalid index
    if (index > list->size || index < 0) {
        return false;
    }

    // Index at the lists edges
    if (index == 0) {
        return ListAppendFirst(list, item);
    }
    if (index == list->size) {
        return ListAppendLast(list, item);
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
void* ListGet(List list, int index) {
    assert(list != NULL);

    // Index outside bounds.
    if (index >= list->size || index < 0) {
        return NULL;
    }

    // Loop through list until index
    list->currentNode = list->firstNode;
    int idx = 0;
    while (list->currentNode != NULL) {
        if (idx == index) {
            return list->currentNode->value;
        }
        list->currentNode = list->currentNode->nextNode;
        idx++;
    }
    
    return NULL;
}


// Removes the first item from the List, returning whether or not it was successful
bool ListRemoveFirst(List list) {
    assert(list != NULL);

    if (list->size == 0) {
        return false;
    }

    // Free first node and set it to second node
    ListNode first = list->firstNode;
    if (list->currentNode == list->firstNode) {
        list->currentNode = list->firstNode->nextNode;
    }
    list->firstNode = list->firstNode->nextNode;

    free(first);    
    list->size--;

    return true;
}

// Removes the last item from the List, returning whether or not it was successful
bool ListRemoveLast(List list) {
    assert(list != NULL);

    if (list->size == 0) {
        return false;
    }

    // Get last node and node before last
    ListNode beforeLast = NULL;
    list->currentNode = list->firstNode;
    while (list->currentNode->nextNode != NULL) {
        beforeLast = list->currentNode;
        list->currentNode = list->currentNode->nextNode;
    }
    
    // Free last node and set the current node to the first node
    ListNode last = list->currentNode;
    list->currentNode = list->firstNode;
    free(last);
    list->size--;

    // If node before last exists, make it the last
    if (beforeLast != NULL) {
        beforeLast->nextNode = NULL;
    }

    return true;
}

// Removes an item from the List, returning whether or not it was successful
bool ListRemove(List list, int index) {
    assert(list != NULL);

    if (list->size == 0) {
        return false;
    }

    // Index outside bounds.
    if (index >= list->size || index < 0) {
        return false;
    }

    if (index == 0) {
        return ListRemoveFirst(list);
    }
    if (index == list->size-1) {
        return ListRemoveLast(list);
    }

    // Loop until right index and the node before that
    ListNode beforeCurrent = NULL;
    list->currentNode = list->firstNode;
    int idx = 0;
    while (idx != index && list->currentNode->nextNode != NULL) {
        beforeCurrent = list->currentNode;
        list->currentNode = list->currentNode->nextNode;
        idx++;
    }
    
    // Remove currentNode from chain.
    if (beforeCurrent != NULL) {
        beforeCurrent->nextNode = list->currentNode->nextNode;
    }
    // Free currentNode.
    free(list->currentNode);
    list->size--;

    return true;
}


// Removes the first item from the List, returning it
void* ListPopFirst(List list) {
    assert(list != NULL);

    if (list->size == 0) {
        return NULL;
    }

    // Free first node and set it to second node
    ListNode first = list->firstNode;
    //printf("first: %p, list->firstNode: %p, list->firstNode->nextNode: %p\n", first, list->firstNode, list->firstNode->nextNode);
    if (list->currentNode == list->firstNode) {
        list->currentNode = list->firstNode->nextNode;
    }
    list->firstNode = list->firstNode->nextNode;
    //printf("first: %p (%s), list->firstNode: %p (%s), list->firstNode->nextNode: %p (%s)\n", first, (char*) first->value, list->firstNode, (char*) list->firstNode->value, list->firstNode->nextNode, (char*) list->firstNode->nextNode->value);

    void* value = first->value;
    free(first);
    list->size--;

    return value;
}

// Removes the last item from the List, returning it
void* ListPopLast(List list) {
    assert(list != NULL);

    if (list->size == 0) {
        return NULL;
    }

    // Get last node and node before last
    ListNode beforeLast = NULL;
    list->currentNode = list->firstNode;
    while (list->currentNode->nextNode != NULL) {
        beforeLast = list->currentNode;
        list->currentNode = list->currentNode->nextNode;
    }
    
    // Free last node and set the current node to the first node
    ListNode last = list->currentNode;
    list->currentNode = list->firstNode;

    void* value = last->value;
    free(last);
    list->size--;

    // If node before last exists, make it the last
    if (beforeLast != NULL) {
        beforeLast->nextNode = NULL;
    }

    return value;
}

// Removes an item from the list, returning it
void* ListPop(List list, int index) {
    assert(list != NULL);

    if (list->size == 0) {
        return NULL;
    }

    // Index outside bounds.
    if (index >= list->size || index < 0) {
        return NULL;
    }

    if (index == 0) {
        return ListPopFirst(list);
    }
    if (index == list->size-1) {
        return ListPopLast(list);
    }

    // Loop until right index and the node before that
    ListNode beforeCurrent = NULL;
    list->currentNode = list->firstNode;
    int idx = 0;
    while (idx != index && list->currentNode->nextNode != NULL) {
        beforeCurrent = list->currentNode;
        list->currentNode = list->currentNode->nextNode;
        idx++;
    }
    
    // Remove currentNode from chain.
    if (beforeCurrent != NULL) {
        beforeCurrent->nextNode = list->currentNode->nextNode;
    }
    // Free currentNode.
    void* value = list->currentNode->value;
    free(list->currentNode);
    list->size--;

    return value;
}


// Moves the pointer to the start
void ListMoveToStart(List list) {
    assert(list != NULL);

    list->currentNode = list->firstNode;
}

// Moves the pointer to the next element (returns false if at the end)
bool ListMoveToNext(List list) {
    assert(list != NULL);

    //if (list->currentNode->nextNode == NULL) {
    //    return false;
    //}

    list->currentNode = list->currentNode->nextNode;
    return true;
}

// Moves the pointer to the end of the list
void ListMoveToEnd(List list) {
    assert(list != NULL);

    while (list->currentNode->nextNode != NULL) {
        list->currentNode = list->currentNode->nextNode;
    }
}

// Returns the current item from the list
void* ListGetCurrent(List list) {
    assert(list != NULL);

    return list->currentNode->value;
}

// Returns whether or not the list has a next element
bool ListHasNext(List list) {
    assert(list != NULL);

    return list->currentNode->nextNode != NULL;
}

// Returns whether or not its safe to operate in the current list node
bool ListCanOperate(List list) {
    assert(list != NULL);

    return list->currentNode != NULL;
}


// Prints the list in the usual format. printFunc (optional) prints the item correctly)
void ListPrint(List list, bool newline, void (*printFunc) (void* item)) {
    assert(list != NULL);
    
    void (*usedPrintFunc) (void* item) = list->printFunc;
    if (printFunc != NULL) {
        usedPrintFunc = printFunc;
    } else {
        usedPrintFunc = list->printFunc;
    }

    if (usedPrintFunc == NULL) {
        printf("NO PRINT FUNCTION SPECIFIED!");
        if (newline) {
            printf("\n");
        }
        return;
    }

    printf("[");
    list->currentNode = list->firstNode;
    while (list->currentNode != NULL) {
        usedPrintFunc(list->currentNode->value);

        list->currentNode = list->currentNode->nextNode;

        if (list->currentNode != NULL) {
            printf(", ");
        }
    }
    
    printf("]");
    if (newline) {
        printf("\n");
    }
}
