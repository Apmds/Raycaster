#include <stdbool.h>

#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap* HashMap;

// Creates a HashMap (hashFunc is the function used fir hashing the key)
HashMap HashMapCreate(int (*hashFunc) (void* key));

// Destroys a HashMap
void HashMapDestroy(HashMap* mapp);


// Puts an item in the HashMap, returning whether or not it was successful
bool HashMapPut(HashMap map, void* key, void* value);

// Returns an item from the HashMap
void* HashMapGet(HashMap map, void* key);

// Removes an item from the HashMap, returning whether or not it was successful
bool HashMapRemove(HashMap map, void* key);

#endif
