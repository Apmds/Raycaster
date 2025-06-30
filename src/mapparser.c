#include <stdlib.h>
#include <assert.h>
#include "mapparser.h"
#include "hashmap.h"
#include "list.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define ERROR_STR "Error parsing map file \"%s\" (Line %d): "

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

// TODO: use this on the value
// Returns a char* that is str with leading and trailing whitespace removed
static char* trim(char* str) {
    // Go forwards while first char is WS
    while (isspace(*str)) {
        str++;
    }

    // If string is only WS, then just return it
    if (*str == '\0') {
        return str;
    }

    // Move endChar backwards until its not WS, then set the char next to it as null-terminator 
    char* endChar = str + strlen(str) - 1;

    while (endChar > str && isspace(*endChar)) {
        endChar--;
    }

    *(endChar+1) = '\0';    // Must end in null-terminator
    
    return str;
}

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

// Helpers
static bool isInt(char* str) {
    char* endChar;
    long i_val = strtol(str, &endChar, 10);
    if (*endChar == '\0') {
        return true;
    }
    return false;
}

static bool isFloat(char* str) {
    char* endChar;
    double i_val = strtod(str, &endChar);
    if (*endChar == '\0') {
        return true;
    }
    return false;
}

static ParserElement ParserTableParseValue(ParserTable table, char* elem_name, char* val, FILE* file, MapParser parser, int* lineNumberp) {
    void* value = NULL;
    ParserTypes type = -1;
    
    
    char first_char = val[0];
    char last_char;
    if (val[strlen(val)-1] == '\n') {
        last_char = val[strlen(val)-2];
    } else {
        last_char = val[strlen(val)-1];
    }

    int n;
    double f;
    if (isInt(val) && sscanf(val, "%d", &n) == 1) {            // Is an integer
        value = malloc(sizeof(int));
        assert(value != NULL);

        type = INT_TYPE;
        *(int *)value = n;
    } else if (isFloat(val) && sscanf(val, "%lf", &f) == 1) {     // Is a float
        value = malloc(sizeof(double));
        assert(value != NULL);

        type = FLOAT_TYPE;
        *(double *)value = f;
    } else if (strcmp(val, "true") == 0 || strcmp(val, "false") == 0) {     // Is a boolean
        value = malloc(sizeof(bool));
        assert(value != NULL);

        type = BOOL_TYPE;
        *(bool *) value = strcmp(val, "true") == 0 ? true : false;
    } else if (first_char == '\"') {        // Is a string
        value = malloc((strlen(val))*sizeof(char));
        assert(value != NULL);
        
        type = STRING_TYPE;
        if (sscanf(val, " \"%[^\"]\" ", (char*)value) != 1) {    // Not single line
            fprintf(stderr, ERROR_STR "Strings must be single line only!\n", parser->filename, *lineNumberp);
            exit(EXIT_FAILURE);
        }
    } else if (first_char == '[') {    // Is a list
        // TODO: parse the list

        //char currchar = *(++val);
        //while (currchar != '\0') {
        //    
        //}
        //
        //
        //
        //if (last_char == ']') {     // Is a single line list
        //    
        //} else {
        //    // Continue parsing until a ']'
        //    char line[500];
        //    while (fgets(line, sizeof(line), file) != NULL) {
        //        
        //    }
        //}
        
    }
    //printf("DEBUG: %p; %d\n", value, type);

    char* ename = calloc(51, sizeof(char)); assert(ename != NULL);
    strncpy(ename, elem_name, 50);

    ParserElement elem = malloc(sizeof(struct parserelement));
    assert(elem != NULL);

    elem->key = ename;
    elem->value = value;     // TODO: Parse the value
    elem->type = type;    // TODO: Parse the value

    HashMapPut(table->elements, ename, elem);
}

ParserResult MapParserParse(MapParser parser) {
    assert(parser != NULL);

    FILE* file = fopen(parser->filename, "r");
    if (file == NULL) {
        perror("Error opening file!");
        exit(EXIT_FAILURE);
    }

    ParserResult res = malloc(sizeof(struct parserresult));
    assert(res != NULL);
    parser->result = res;
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

        // TODO: substituir para ler char a char
        char table_name[51];
        if (sscanf(line, " [%50[^]]] ", table_name) == 1) {   // New table
            if (HashMapContains(res->tables, table_name)) {
                fprintf(stderr, ERROR_STR "Duplicate table name %s.\n", parser->filename, lineNumber, table_name);
                exit(EXIT_FAILURE);
            }

            char* tname = calloc(51, sizeof(char)); assert(tname != NULL);
            strncpy(tname, table_name, 50);

            ParserTable table = ParserTableCreate(tname);
            assert(table != NULL);
            HashMapPut(res->tables, tname, table);
            currentTable = table;
            continue;
        }

        
        bool inString = false;  // True if currently inside of a string
        int listNesting = 0;    // Number of list nesting
        int objNesting = 0;    // Number of object nesting
        int splitIdx = -1;  // Index in line of key-value separation
        for (int i = 0; line[i] != '\0'; i++) {
            char c = line[i];
            
            if (c == '\"') { inString = !inString; }
            if (c == '[') { listNesting++; }
            if (c == '{') { objNesting++; }
            if (c == ']') { listNesting--; }
            if (c == '}') { objNesting--; }
            
            if (c == ':' && listNesting == 0 && objNesting == 0 && !inString) {
                splitIdx = i;
                break;
            }
        }
        
        if (splitIdx <= 0) { // Also works if the first char is :
            fprintf(stderr, ERROR_STR "Wrong file formatting. Must be <key> : <value>\n", parser->filename, lineNumber);
            exit(EXIT_FAILURE);
        }

        // New key value pair

        // Get name and value strings
        char elem_name[splitIdx+1];
        strncpy(elem_name, line, splitIdx);
        elem_name[splitIdx] = '\0';

        char val[strlen(line)-splitIdx+1];
        strncpy(val, line+splitIdx+1, strlen(line)-splitIdx);
        val[strlen(line)-splitIdx] = '\0';

        char* trimmed_name = trim(elem_name);
        char* trimmed_val = trim(val);

        if (currentTable == NULL) {     // Defined outside of a table
            fprintf(stderr, ERROR_STR "Element defined outside of a table %s.\n", parser->filename, lineNumber, table_name);
            exit(EXIT_FAILURE);
        }
        if (HashMapContains(currentTable->elements, trimmed_name)) {    // Element already defined here
            fprintf(stderr, ERROR_STR "Duplicate element name %s.\n", parser->filename, lineNumber, trimmed_name);
            exit(EXIT_FAILURE);
        }

        ParserTableParseValue(currentTable, trimmed_name, trimmed_val, file, parser, &lineNumber);
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
    free(table->name);
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
