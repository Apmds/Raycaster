#include <stdbool.h>

#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap* HashMap;
typedef struct hashmapi* HashMapIterator;


// Creates a HashMap (hashFunc is the function used fir hashing the key)
HashMap HashMapCreate(int size, unsigned int (*hashFunc) (void* key), bool (*compFunc) (void* key1, void* key2));

// Destroys a HashMap
void HashMapDestroy(HashMap* mapp);


// Puts an item in the HashMap, returning whether or not it was successful
bool HashMapPut(HashMap map, void* key, void* value);

// Returns an item from the HashMap
void* HashMapGet(const struct hashmap* map, void* key);

// Returns whether or not the HashMap contains a value for the given key
bool HashMapContains(const struct hashmap* map, void* key);

// Removes an item from the HashMap, returning whether or not it was successful
bool HashMapRemove(const struct hashmap* map, void* key);

// Removes an item from the HashMap, returning it
void* HashMapPop(const struct hashmap* map, void* key);

// Prints the map in usual format. printFunc (optional) prints the item correctly)
void HashMapPrint(const struct hashmap* map, bool newline, void (*printFunc) (void* key, void* value));


// Iterating functions

// Returns an iterator for this hashmap
HashMapIterator HashMapGetIterator(HashMap map);

void HashMapIterDestroy(HashMapIterator* iterp);

// Goes to the next element/key in the map (returns false if at the end)
bool HashMapIterGoToNext(HashMapIterator iter);

// Returns whether or not its safe to operate in the current element
bool HashMapIterCanOperate(const struct hashmapi* iter);

// Returns the current key the iterator is in
void* HashMapIterGetCurrentKey(HashMapIterator iter);

// Returns the current value the iterator is in
void* HashMapIterGetCurrentValue(HashMapIterator iter);


#endif
