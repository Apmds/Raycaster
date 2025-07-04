#include <stdbool.h>
#include "hashmap.h"

#ifndef MAPPARSER_H
#define MAPPARSER_H

typedef struct mapparser* MapParser;

typedef struct parserresult* ParserResult;

typedef struct parsertable* ParserTable;

typedef struct parserelement* ParserElement;

typedef enum {
    BOOL_TYPE,          // true or false                        --> bool
    STRING_TYPE,        // "<something>"                        --> char*
    INT_TYPE,           // integers                             --> int
    FLOAT_TYPE,         // floating point values                --> double
    LIST_TYPE,          // [<item1>, <item2>, ...]              --> List (ArrayList)
    TABLE_TYPE,         // {<key> : <value>, ...}               --> HashMap
} ParserTypes;

// Generates a parser for this file.
MapParser MapParserCreate(const char* filename);

// Destroys a parser
void MapParserDestroy(MapParser* parserp);

// Executes a parser and returns the result (must be explicitly freed afterwards).
ParserResult MapParserParse(MapParser parser);

// Returns the result from a parser (NULL if MapParserParse not called).
ParserResult MapParserGetResult(MapParser parser);


// Returns whether the parser table associated with tableName exists or not.
bool ParserResultHasTable(ParserResult res, char* tableName);

// Returns the parser table associated with tableName (NULL if invalid or unknown table name).
ParserTable ParserResultGetTable(ParserResult res, char* tableName);

// Returns the name associated with table.
char* ParserTableGetName(ParserTable table);

// Returns whether the element associated with elementName exists or not.
bool ParserTableHasElement(ParserTable table, char* elementName);

// Returns the element associated with elementName (NULL if invalid or unknown element name).
ParserElement ParserTableGetElement(ParserTable table, char* elementName);

// Returns a hashmap that associates element names to elements.
HashMap ParserTableGetHashMap(ParserTable table);

// Returns the key of an element.
char* ParserElementGetKey(ParserElement elem);

// Returns the value of an element. To know what to cast it to, check ParserElementGetType.
void* ParserElementGetValue(ParserElement elem);

// Returns the type of the element in the parser.
ParserTypes ParserElementGetType(ParserElement elem);

// Destroys a parserresult
void ParserResultDestroy(ParserResult* resp);

#endif