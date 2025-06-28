#include <stdlib.h>
#include <assert.h>
#include "mapparser.h"
#include "hashmap.h"

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


void ParserResultDestroy(ParserResult* resp) {
    assert(resp != NULL);
    assert(*resp != NULL);

    ParserResult res = *resp;

    // TODO: free

    free(res);

    *resp = NULL;
}
