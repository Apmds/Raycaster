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
    unsigned int (*hashFunc) (void* key);       // Key hashing function
    bool (*compFunc) (void* key1, void* key2);  // Key comparing function
};

struct hashmapi {
    HashMap map;
    int currIdxList;    // Index of current list in hashmap
    int currIdxKey;     // Index of current item in current list
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
bool HashMapPut(CHashMap map, void* key, void* value) {
    assert(map != NULL);
    assert(key != NULL);

    // Get list index
    unsigned int hash_val = map->hashFunc(key) % map->size;

    // If item does not exist, create new element
    if (!HashMapContains(map, key)) {
        HashMapElement element = malloc(sizeof(hashmap_elem));
        assert(element != NULL);
    
        element->key = key;
        element->value = value;
        ListAppendFirst(map->values[hash_val], (void*) element);
        return true;
    }
        
    // Update item if exists
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

    return false;
}

// Returns an item from the HashMap
void* HashMapGet(CHashMap map, void* key) {
    assert(map != NULL);
    assert(key != NULL);

    // Get list index
    int hash_val = map->hashFunc(key) % map->size;
    
    List list = map->values[hash_val];

    void* value = NULL;

    ListMoveToStart(list);
    while (ListCanOperate(list)) {
        HashMapElement element = (HashMapElement) ListGetCurrent(list);

        bool compVal = (map->compFunc != NULL) ? 
            map->compFunc(element->key, key) : element->key == key;

        if (compVal) {
            value = element->value;
            break;
        }

        ListMoveToNext(list);
    }

    return value;
}

// Returns whether or not the HashMap contains a value for the given key
bool HashMapContains(CHashMap map, void* key) {
    assert(map != NULL);

    return HashMapGet(map, key) != NULL;
}

// Removes an item from the HashMap, returning whether or not it was successful
bool HashMapRemove(CHashMap map, void* key) {
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
void* HashMapPop(CHashMap map, void* key) {
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
void HashMapPrint(CHashMap map, bool newline, void (*printFunc) (void* key, void* value)) {
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
    
    printf("\b\b  \b\b");   // Deletes the last 2 chars (the last ", ")
    printf("}");
    if (newline) {
        printf("\n");
    }
}


// Iterating functions

// Returns an iterator for this hashmap
HashMapIterator HashMapGetIterator(HashMap map) {
    assert(map != NULL);

    HashMapIterator iter = malloc(sizeof(struct hashmapi));
    assert(iter != NULL);

    iter->map = map;
    iter->currIdxKey = -1;
    iter->currIdxList = 0;
    HashMapIterGoToNext(iter);  // Move to the first available spot

    return iter;
}

// Destroys an iterator
void HashMapIterDestroy(HashMapIterator* iterp) {
    assert(iterp != NULL);
    assert(*iterp != NULL);

    HashMapIterator iter = *iterp;

    

    free(iter);
    *iterp = NULL;
}

// Goes to the next element/key in the map (returns false if at the end)
bool HashMapIterGoToNext(HashMapIterator iter) {
    assert(iter != NULL);
    assert(iter->map != NULL);
    
    // If its past the last list, its at the end
    if (iter->currIdxList >= iter->map->size) {
        return false;
    }
    
    CList currentList = iter->map->values[iter->currIdxList];

    // If is at end of current list, go to next
    if (++iter->currIdxKey >= ListGetSize(currentList)) {
        iter->currIdxKey = -1;
        iter->currIdxList++;
        return HashMapIterGoToNext(iter);
    }

    return true;
}

// Returns whether or not its safe to operate in the current element
bool HashMapIterCanOperate(CHashMapIterator iter) {
    assert(iter != NULL);
    assert(iter->map != NULL);

    // Is in a valid list and with a valid idx inside the current list
    return iter->currIdxList < iter->map->size && iter->currIdxKey < ListGetSize(iter->map->values[iter->currIdxList]);
}

// Returns the current key the iterator is in
void* HashMapIterGetCurrentKey(CHashMapIterator iter) {
    assert(iter != NULL);
    assert(iter->map != NULL);

    return HashMapIterCanOperate(iter) ? ((HashMapElement) ListGet(iter->map->values[iter->currIdxList], iter->currIdxKey))->key : NULL;
}

// Returns the current value the iterator is in
void* HashMapIterGetCurrentValue(CHashMapIterator iter) {
    assert(iter != NULL);
    assert(iter->map != NULL);

    return HashMapIterCanOperate(iter) ? ((HashMapElement) ListGet(iter->map->values[iter->currIdxList], iter->currIdxKey))->value : NULL;
}
