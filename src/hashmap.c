#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "hashmap.h"
#include "list.h"

typedef struct {
    void* key;
    void* value;
} hashmap_elem;

typedef struct hashmap_elem* HashMapElement;

struct hashmap {
    int size;
    List* values;       // HashMapElement lists
    int (*hashFunc) (void* key);
};

// Creates a HashMap (hashFunc is the function used fir hashing the key)
HashMap HashMapCreate(int size, int (*hashFunc) (void* key)) {
    assert(hashFunc != NULL);
    assert(size > 0);

    HashMap map = malloc(sizeof(struct hashmap));
    assert(map != NULL);

    map->hashFunc = hashFunc;
    map->size = size;
    
    // Create lists
    map->values = malloc(sizeof(List)*5);
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
        ListDestroy(&map->values[i]);
    }
    free(map->values);

    free(map);
    *mapp = NULL;
}


// Puts an item in the HashMap, returning whether or not it was successful
bool HashMapPut(HashMap map, void* key, void* value);

// Returns an item from the HashMap
void* HashMapGet(HashMap map, void* key);

// Removes an item from the HashMap, returning whether or not it was successful
bool HashMapRemove(HashMap map, void* key);
