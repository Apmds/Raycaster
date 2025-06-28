#include <stdlib.h>
#include <assert.h>
#include "mapparser.h"
#include "hashmap.h"
#include "list.h"
#include <stdio.h>
#include <string.h>

// djb2 hash
static unsigned int djb2hash(void* key) {
    char* str = (char*) key;

    int hash = 5381;
    int c;

    while (c = *str++)
       hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static bool hashmapstrcmp(void* key1, void* key2) {
    return strcmp((char*) key1, (char*) key2) == 0;
}

struct mapparser {
    char* filename;
    ParserResult result;
};

struct parserresult {
    HashMap tables;     // Map that associates table names (char*) to tables
};

struct parsertable {
    char* name;         // The name. DO NOT FREE ON DESTROY (destroying parserresult will free this)
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

// INTERNAL: creates a parser table
static ParserTable ParserTableCreate(char* name) {

    ParserTable table = malloc(sizeof(struct parsertable));
    assert(table != NULL);

    table->name = name;
    table->elements = HashMapCreate(5, djb2hash, hashmapstrcmp);

    return table;
}

static void ParserTablePutElement(ParserTable table, char* key, void* value, ParserTypes type);

ParserResult MapParserParse(MapParser parser) {
    assert(parser != NULL);

    FILE* file = fopen(parser->filename, "r");
    if (file == NULL) {
        perror("Error opening file!");
        exit(EXIT_FAILURE);
    }

    ParserResult res = malloc(sizeof(struct parserresult));
    assert(res != NULL);
    res->tables = HashMapCreate(5, djb2hash, hashmapstrcmp);
    
    char line[500];
    int lineNumber = 0;
    ParserTable currentTable = NULL;

    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;

        // Ignore empty lines
        int line_size = strlen(line);
        if (line_size == 0 || line[0] == '\n' || line_size == strspn(line, " \r\n\t")) {
            continue;
        }

        char table_name[51];
        if (sscanf(line, " [%50[^]]] ", table_name) == 1) {   // New table
            if (HashMapContains(res->tables, table_name)) {
                printf("Error parsing map file \"%s\" (Line %d): Duplicate table name %s.\n", parser->filename, line, table_name);
                exit(EXIT_FAILURE);
            }

            char* tname = calloc(51, sizeof(char)); assert(tname != NULL);
            strncpy(tname, table_name, 50);

            ParserTable table = ParserTableCreate(tname);
            HashMapPut(res->tables, tname, table);
            currentTable = table;
            continue;
        }

        char elem_name[51];
        char val[201];
        if (sscanf(line, " %50s : %200s ", elem_name, val) == 2) {  // New value
            if (currentTable == NULL) {     // Defined outside of a table
                printf("Error parsing map file \"%s\" (Line %d): Element defined outside of a table %s.\n", parser->filename, line, table_name);
                exit(EXIT_FAILURE);
            }
            if (HashMapContains(currentTable->elements, elem_name)) {    // Element already defined here
                printf("Error parsing map file \"%s\" (Line %d): Duplicate element name %s.\n", parser->filename, line, elem_name);
                exit(EXIT_FAILURE);
            }

            // TODO: parse the value

            char* ename = calloc(51, sizeof(char)); assert(ename != NULL);
            strncpy(ename, elem_name, 50);

            ParserElement elem = malloc(sizeof(struct parserelement));
            assert(elem != NULL);

            elem->key = ename;
            //elem->value =     // TODO: Parse the value
            //elem->type =      // TODO: Parse the value

            HashMapPut(currentTable->elements, ename, elem);
            continue;
        }
    }

    fclose(file);

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
