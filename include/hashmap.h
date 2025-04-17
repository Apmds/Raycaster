#include <stdbool.h>

#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap* HashMap;

// Creates a HashMap (compareFunc must return a value > 0 if key1 > key2, 0 if key1 = key2, and a value < 0 if key1 < key2)
HashMap HashMapCreate(int (*compareFunc) (void* key1, void* key2));

// Destroys a HashMap
void HashMapDestroy(HashMap* mapp);


// Puts an item in the HashMap, returning whether or not it was successful
bool HashMapPut(HashMap map, void* key, void* value);

// Returns an item from the HashMap
void* HashMapGet(HashMap map, void* key);

// Removes an item from the HashMap, returning whether or not it was successful
bool HashMapRemove(HashMap map, void* key);

#endif
