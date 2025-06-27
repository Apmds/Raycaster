#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "hashmap.h"
#include "list.h"

typedef struct hashmap_elem {
    void* key;
    void* value;
} hashmap_elem;

typedef struct hashmap_elem* HashMapElement;

struct hashmap {
    int size;
    List* values;       // HashMapElement lists
    unsigned int (*hashFunc) (void* key);
    bool (*compFunc) (void* key1, void* key2);
};

// Creates a HashMap (hashFunc is the function used fir hashing the key) (compFunc if used for comparing 2 keys. If not given, it compares pointers)
HashMap HashMapCreate(int size, unsigned int (*hashFunc) (void* key), bool (*compFunc) (void* key1, void* key2)) {
    assert(hashFunc != NULL);
    assert(size > 0);

    HashMap map = malloc(sizeof(struct hashmap));
    assert(map != NULL);

    map->hashFunc = hashFunc;
    map->compFunc = compFunc;
    map->size = size;
    
    // Create lists
    map->values = malloc(sizeof(List)*size);
    for (int i = 0; i < size; i++) {
        map->values[i] = ListCreate(NULL);
    }

    return map;
}

// Destroys a HashMap
void HashMapDestroy(HashMap* mapp) {
    assert(mapp != NULL);
    assert(*mapp != NULL);

    HashMap map = *mapp;

    // Destroy the lists
    for (int i = 0; i < map->size; i++) {
        // Destroy the lists' items
        List list = map->values[i];
        
        ListMoveToStart(list);
        while (ListCanOperate(list)) {
            free(ListGetCurrent(list));

            ListMoveToNext(list);
        }
        
        ListDestroy(&list);
    }
    free(map->values);

    free(map);
    *mapp = NULL;
}


// Puts an item in the HashMap, returning whether or not it was successful
bool HashMapPut(HashMap map, void* key, void* value) {
    assert(map != NULL);
    assert(key != NULL);

    // Get list index
    unsigned int hash_val = map->hashFunc(key) % map->size;

    // Update item if exists
    if (HashMapContains(map, key)) {
        List list = map->values[hash_val];

        ListMoveToStart(list);
        while (ListCanOperate(list)) {
            HashMapElement element = (HashMapElement) ListGetCurrent(list);

            if (map->compFunc != NULL) {
                if (map->compFunc(element->key, key)) {
                    element->value = value;
                    return true;
                }
            } else {
                if (element->key == key) {
                    element->value = value;
                    return true;
                }
            }

            ListMoveToNext(list);
        }

    }

    HashMapElement element = malloc(sizeof(hashmap_elem));
    assert(element != NULL);

    element->key = key;
    element->value = value;
    ListAppendFirst(map->values[hash_val], (void*) element);
    return true;
}

// Returns an item from the HashMap
void* HashMapGet(HashMap map, void* key) {
    assert(map != NULL);
    assert(key != NULL);

    // Get list index
    int hash_val = map->hashFunc(key) % map->size;
    
    List list = map->values[hash_val];

    void* value = NULL;

    ListMoveToStart(list);
    while (ListCanOperate(list)) {
        HashMapElement element = (HashMapElement) ListGetCurrent(list);

        if (map->compFunc != NULL) {
            if (map->compFunc(element->key, key)) {
                value = element->value;
                break;
            }
        } else {
            if (element->key == key) {
                value = element->value;
                break;
            }
        }
            

        ListMoveToNext(list);
    }

    return value;
}

// Returns whether or not the HashMap contains a value for the given key
bool HashMapContains(HashMap map, void* key) {
    return HashMapGet(map, key) != NULL;
}

// Removes an item from the HashMap, returning whether or not it was successful
bool HashMapRemove(HashMap map, void* key) {
    assert(map != NULL);

    // Get list index
    int hash_val = map->hashFunc(key) % map->size;
    
    List list = map->values[hash_val];
    ListMoveToStart(list);
    int i = 0;
    while (ListCanOperate(list)) {
        HashMapElement element = ListGetCurrent(list);
        
        if (map->compFunc != NULL) {
            if (map->compFunc(element->key, key)) {
                ListPop(list, i);
                return true;
            }
        } else {
            if (element->key == key) {
                ListPop(list, i);
                return true;
            }
        }

        i++;
        ListMoveToNext(list);
    }

    return false;
}

// Removes an item from the HashMap, returning it
void* HashMapPop(HashMap map, void* key) {
    assert(map != NULL);

    // Get list index
    int hash_val = map->hashFunc(key) % map->size;
    
    List list = map->values[hash_val];
    ListMoveToStart(list);
    int i = 0;
    while (ListCanOperate(list)) {
        HashMapElement element = ListGetCurrent(list);
        
        if (map->compFunc != NULL) {
            if (map->compFunc(element->key, key)) {
                void* val = ListPop(list, i);
                return val;
            }
        } else {
            if (element->key == key) {
                void* val = ListPop(list, i);
                return val;
            }
        }

        i++;
        ListMoveToNext(list);
    }

    return NULL;
}

// Prints the map in usual format. printFunc (optional) prints the item correctly)
void HashMapPrint(HashMap map, bool newline, void (*printFunc) (void* key, void* value)) {
    assert(map != NULL);

    if (printFunc == NULL) {
        printf("NO PRINT FUNCTION SPECIFIED!");
        if (newline) {
            printf("\n");
        }
        return;
    }

    printf("{");
    for (int i = 0; i < map->size; i++) {
        List list = map->values[i];
        
        ListMoveToStart(list);
        while (ListCanOperate(list)) {
            HashMapElement element = ListGetCurrent(list);
            printFunc(element->key, element->value);
            printf(", ");
            ListMoveToNext(list);
        }
    }
    // Delete the last 2 chars (the last ", ")
    printf("\b\b  \b\b");
    printf("}");
    if (newline) {
        printf("\n");
    }
}
