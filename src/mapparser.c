#include <stdlib.h>
#include <assert.h>
#include "mapparser.h"
#include "hashmap.h"
#include "list.h"

struct mapparser {
    char* filename;
    ParserResult result;
};

struct parserresult {
    HashMap tables;     // Map that associates table names (char*) to tables
};

struct parsertable {
    HashMap elements;   // Map that associates element names (char*) to elements
};

struct parserelement {
    ParserTypes type;   // Type of the element value
    char* key;          // Element name
    void* value;        // Element value
};

// INTERNAL: free a ParserTable (not meant to be freed outside of ParserResultDestroy and ParserElementDestroy)
static void ParserTableDestroy(ParserTable* tablep);

// INTERNAL: free a ParserElement (not meant to be freed outside of ParserTableDestroy)
static void ParserElementDestroy(ParserElement* elemp);

MapParser MapParserCreate(char* filename) {
    assert(filename != NULL);

    MapParser parser = malloc(sizeof(struct mapparser));
    assert(parser != NULL);

    parser->filename = filename;
    parser->result = NULL;

    return parser;
}

void MapParserDestroy(MapParser* parserp) {
    assert(parserp != NULL);
    assert(*parserp != NULL);

    MapParser parser = *parserp;

    // TODO: free

    free(parser);

    *parserp = NULL;
}

ParserResult MapParserParse(MapParser parser) {
    assert(parser != NULL);

    // TODO: parse

    return parser->result;
}

ParserResult MapParserGetResult(MapParser parser) {
    assert(parser != NULL);

    return parser->result;
}


// Returns the parser table associated with tableName (NULL if invalid or unknown table name).
ParserTable ParserResultGetTable(ParserResult res, char* tableName) {
    assert(res != NULL);

    if (!HashMapContains(res->tables, tableName)) {
        return NULL;
    }

    return (ParserTable) HashMapGet(res->tables, tableName);
}

// Returns the elemnt associated with elementName (NULL if invalid or unknown element name).
ParserElement ParserTableGetElement(ParserTable table, char* elementName) {
    assert(table != NULL);

    if (!HashMapContains(table->elements, elementName)) {
        return NULL;
    }

    return (ParserElement) HashMapGet(table->elements, elementName);
}

// Returns the key of an element.
char* ParserElementGetKey(ParserElement elem) {
    assert(elem != NULL);

    return elem->key;
}

// Returns the value of an element. To know what to cast it to, check ParserElementGetType.
void* ParserElementGetValue(ParserElement elem) {
    assert(elem != NULL);

    return elem->value;
}

// Returns the type of the element in the parser.
ParserTypes ParserElementGetType(ParserElement elem) {
    assert(elem != NULL);

    return elem->type;
}

// INTERNAL: free a ParserTable (not meant to be freed outside of ParserResultDestroy)
static void ParserTableDestroy(ParserTable* tablep) {
    assert(tablep != NULL);
    assert(*tablep != NULL);

    ParserTable table = *tablep;

    HashMapIterator iter = HashMapGetIterator(table->elements);
    while (HashMapIterCanOperate(iter)) {
        // TODO: Maybe also free the keys idk
        ParserElement element = (ParserElement) HashMapIterGetCurrentValue(iter);

        ParserElementDestroy(&element);

        HashMapIterGoToNext(iter);
    }
    HashMapIterDestroy(&iter);
    HashMapDestroy(&(table->elements));

    free(table);

    *tablep = NULL;
}

// INTERNAL: free a ParserElement (not meant to be freed outside of ParserTableDestroy)
static void ParserElementDestroy(ParserElement* elementp) {
    assert(elementp != NULL);
    assert(*elementp != NULL);

    ParserElement element = *elementp;

    free(element->key);

    // Free the value based on type
    switch (element->type) {
        // Simple freeing, nothing more
        case BOOL_TYPE:
        case INT_TYPE:
        case FLOAT_TYPE:
        case STRING_TYPE:
            free(element->value);
            break;

        // Must destroy the list and free the values
        case LIST_TYPE:
            List vals = (List) element->value;

            ListMoveToStart(vals);
            while (ListCanOperate(vals)) {
                ParserElement elem = (ParserElement) ListGetCurrent(vals);
                ParserElementDestroy(&elem);

                ListMoveToNext(vals);
            }
            

            ListDestroy(&vals);
            break;
        
        // Must destroy the table
        case TABLE_TYPE:
            ParserTable table = (ParserTable) element->value;
            ParserTableDestroy(&table);
            break;
    }

    free(element);

    *elementp = NULL;
}

void ParserResultDestroy(ParserResult* resp) {
    assert(resp != NULL);
    assert(*resp != NULL);

    ParserResult res = *resp;

    // TODO: free
    HashMapIterator iter = HashMapGetIterator(res->tables);
    while (HashMapIterCanOperate(iter)) {
        // TODO: Maybe also free the keys idk
        ParserTable table = (ParserTable) HashMapIterGetCurrentValue(iter);

        ParserTableDestroy(&table);

        HashMapIterGoToNext(iter);
    }
    HashMapIterDestroy(&iter);
    HashMapDestroy(&(res->tables));

    free(res);

    *resp = NULL;
}
