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
    const char* filename;
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

static int getSplitIdx(char* line) {
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

    return splitIdx;
}

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

// Parses a key-value pair
static ParserElement parseKVPair(char* kvpair, MapParser parser, int lineNumber);

static ParserElement parseValue(char* elem_name, char* val, MapParser parser, int lineNumber);

MapParser MapParserCreate(const char* filename) {
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

    free(parser);

    *parserp = NULL;
}

// INTERNAL: creates a parser table
static ParserTable ParserTableCreate(char* name) {
    assert(name != NULL);

    ParserTable table = malloc(sizeof(struct parsertable));
    assert(table != NULL);

    table->name = name;
    table->elements = HashMapCreate(5, djb2hash, hashmapstrcmp);

    return table;
}

static bool valueParsable(char* val) {
    assert(val != NULL);

    char first_char = val[0];
    char last_char = val[strlen(val)-1];

    bool lst = false;
    bool obj = false;

    // TODO: do a better obj check maybe
    if (first_char == '[') {
        int nesting = 1;
        char* c = val+1;
        while (*c != '\0') {
            if (*c == '[') {
                nesting++;
            }
            if (*c == ']') {
                nesting--;
            }

            c++;
        }
        lst = nesting == 0;
    }
    if (first_char == '{') {
        int nesting = 1;
        char* c = val+1;
        while (*c != '\0') {
            if (*c == '{') {
                nesting++;
            }
            if (*c == '}') {
                nesting--;
            }

            c++;
        }
        obj = nesting == 0;
    }

    return isInt(val)
        || isFloat(val)
        || (strcmp(val, "true") == 0 || strcmp(val, "false") == 0)
        || (first_char == '\"' && last_char == '\"')
        || lst
        || obj;
}

// Parses a key-value pair
static ParserElement parseKVPair(char* kvpair, MapParser parser, int lineNumber) {
    assert(parser != NULL);
    assert(kvpair != NULL);

    int splitIdx = getSplitIdx(kvpair);
                    
    if (splitIdx <= 0) { // Also works if the first char is :
        fprintf(stderr, ERROR_STR "Invalid table element formatting. Must be <key> : <value>\n", parser->filename, lineNumber);
        exit(EXIT_FAILURE);
    }
    
    // New key value pair
    
    // Get name and value strings
    char table_elem_name[splitIdx+1];
    strncpy(table_elem_name, kvpair, splitIdx);
    table_elem_name[splitIdx] = '\0';
    
    char table_val[strlen(kvpair)-splitIdx+1];
    strncpy(table_val, kvpair+splitIdx+1, strlen(kvpair)-splitIdx);
    table_val[strlen(kvpair)-splitIdx] = '\0';
    
    char* trimmed_name = trim(table_elem_name);
    char* trimmed_val = trim(table_val);
    
    ParserElement parserElem = parseValue(trimmed_name, trimmed_val, parser, lineNumber);

    return parserElem;
}

static ParserElement parseValue(char* elem_name, char* val, MapParser parser, int lineNumber) {
    assert(parser != NULL);
    assert(val != NULL);
    assert(elem_name != NULL);

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
            fprintf(stderr, ERROR_STR "Strings must be single line only!\n", parser->filename, lineNumber);
            exit(EXIT_FAILURE);
        }
    } else if (first_char == '[') {    // Is a list        
        char* line = trim(val+1);

        type = LIST_TYPE;
        value = ListCreate(NULL);
        int nesting = 1;

        first_char = line[0];
        last_char = line[strlen(line)-1];
        char* last_comma = line;    // Pointer to last comma
        
        bool last_element_lst = false;  // True if last element found was a list (next ',' can be ignored)
        char* c = line;
        while (*c != '\0') {
            if (*c == ',')  {   // New element found
                if (last_element_lst) {
                    last_element_lst = false;
                    last_comma = c+1;
                } else {
                    char* valstr = malloc(((int) (c-last_comma+1)) * sizeof(char)); assert(valstr != NULL);
                    strncpy(valstr, last_comma, ((int) (c-last_comma)));
                    valstr[((int) (c-last_comma))] = '\0';
                    // TODO: maybe change "ListElement" to another name or nah idk
                    ListAppendLast(value, parseValue("ListElement", trim(valstr), parser, lineNumber));                    
                    free(valstr);
                    last_comma = c+1;
                }
            }

            if (*c == '[') {    // There's a list inside (must parse it first)
                
                char* valstr = c;

                // Finds the ']' that correlates with the '[' from *c
                char* closeptr = c+1;
                int start_nesting = nesting++;
                while (start_nesting != nesting && *closeptr != '\0') {
                    if (*closeptr == '[') {
                        nesting++;
                    }
                    if (*closeptr == ']') {
                        nesting--;
                    }

                    closeptr++;
                }

                // The nesting itself should be 1 value larger (the last calculation sets nesting to start_nesting)
                nesting++;

                
                bool oneLine = closeptr != NULL;

                if (!oneLine) {
                    fprintf(stderr, ERROR_STR "Invalid value %s.\n", parser->filename, lineNumber, val);
                    exit(EXIT_FAILURE);
                }
                
                if (oneLine) {   // List ends this line (valstr is only the part from here to next line)
                    valstr = malloc( ((int) (closeptr-c+1)) * sizeof(char));
                    assert(valstr != NULL);
                    strncpy(valstr, c, ((int) (closeptr-c)));
                    valstr[((int) (closeptr-c))] = '\0';
                    nesting--;
                }

                // TODO: maybe change "ListElement" to another name or nah idk
                ListAppendLast(value, parseValue("ListElement", valstr, parser, lineNumber));

                if (oneLine) {
                    c = closeptr; // c advances past the sublist
                    last_element_lst = true;
                    free(valstr);
                    continue;
                } else {
                    c = line+strlen(line); // c goes to the end of the line
                    continue;
                }
            }
            
            if (*c == ']') {
                nesting--;
                if (last_element_lst) {
                    last_element_lst = false;
                } else {
                    // Parse last list element
                    char* valstr = malloc(((int) (c-last_comma+1)) * sizeof(char)); assert(valstr != NULL);
                    strncpy(valstr, last_comma, ((int) (c-last_comma)));
                    valstr[((int) (c-last_comma))] = '\0';
                    
                    
                    // TODO: maybe change "ListElement" to another name or nah idk
                    ListAppendLast(value, parseValue("ListElement", trim(valstr), parser, lineNumber));                    
                    free(valstr);
                    last_comma = c+1;
                    
                    last_element_lst = true;
                    
                }
                if (nesting == 0) {
                    break;
                }
            }

            if (*c == '{') {    // Table inside (parse it first)
                char* valstr = c;
                    
                // Finds the '}' that correlates with the '{' from *c
                char* closeptr = c+1;
                int start_nesting = nesting++;
                while (start_nesting != nesting && *closeptr != '\0') {
                    if (*closeptr == '{') {
                        nesting++;
                    }
                    if (*closeptr == '}') {
                        nesting--;
                    }

                    closeptr++;
                }

                // The nesting itself should be 1 value larger (the last calculation sets nesting to start_nesting)
                nesting++;
                
                bool oneLine = closeptr != NULL;
                
                if (!oneLine) {
                    fprintf(stderr, ERROR_STR "Invalid value %s.\n", parser->filename, lineNumber, val);
                    exit(EXIT_FAILURE);
                }

                if (oneLine) {   // Table ends this line (valstr is only the part from here to next line)
                    valstr = malloc( ((int) (closeptr-c+1)) * sizeof(char));
                    assert(valstr != NULL);
                    strncpy(valstr, c, ((int) (closeptr-c)));
                    valstr[((int) (closeptr-c))] = '\0';
                    
                    nesting--;
                }
                
                ListAppendLast(value, parseValue("ListElement", valstr, parser, lineNumber));

                if (oneLine) {
                    c = closeptr; // c advances past the subtable
                    last_element_lst = true;
                    free(valstr);
                    continue;
                } else {
                    c = line+strlen(line); // c goes to the end of the line
                    continue;
                }
            }
            
            c++;
        }
    } else if (first_char == '{') {     // Is a table
        char* line = trim(val+1);

        type = TABLE_TYPE;

        value = HashMapCreate(5, djb2hash, hashmapstrcmp);

        int nesting = 1;

        first_char = line[0];
        last_char = line[strlen(line)-1];
        char* last_comma = line;    // Pointer to last comma
        
        bool last_element_lst = false;  // True if last element found was a list (next ',' can be ignored)
        char* c = line;
        while (*c != '\0') {
            if (*c == ',')  {   // New element found
                if (last_element_lst) {
                    last_element_lst = false;
                    last_comma = c+1;
                } else {
                    // Get the key value pair
                    char* keyval_pair = malloc(((int) (c-last_comma+1)) * sizeof(char)); assert(keyval_pair != NULL);
                    strncpy(keyval_pair, last_comma, ((int) (c-last_comma)));
                    keyval_pair[((int) (c-last_comma))] = '\0';
                    
                    ParserElement parserElem = parseKVPair(keyval_pair, parser, lineNumber);
                    if (HashMapContains((HashMap) value, parserElem->key)) {
                        fprintf(stderr, ERROR_STR "Element with key \"%s\" already exists in this table!\n", parser->filename, lineNumber, parserElem->key);
                        exit(EXIT_FAILURE);
                    }

                    HashMapPut((HashMap) value, parserElem->key, parserElem);
                    
                    free(keyval_pair);
                    last_comma = c+1;
                }
            }
                
            if (*c == '{') {    // There's a table inside (must parse it first)
                char* keyval_pair = c;
                    
                // Finds the '}' that correlates with the '{' from *c
                char* closeptr = c+1;
                int start_nesting = nesting++;
                while (start_nesting != nesting && *closeptr != '\0') {
                    if (*closeptr == '{') {
                        nesting++;
                    }
                    if (*closeptr == '}') {
                        nesting--;
                    }

                    closeptr++;
                }

                // The nesting itself should be 1 value larger (the last calculation sets nesting to start_nesting)
                nesting++;
                
                bool oneLine = closeptr != NULL;
                
                if (!oneLine) {
                    fprintf(stderr, ERROR_STR "Invalid value %s.\n", parser->filename, lineNumber, val);
                    exit(EXIT_FAILURE);
                }

                if (oneLine) {   // Table ends this line (valstr is only the part from here to next line)
                    keyval_pair = malloc( ((int) (closeptr-last_comma+1)) * sizeof(char));
                    assert(keyval_pair != NULL);
                    strncpy(keyval_pair, last_comma, ((int) (closeptr-last_comma)));
                    keyval_pair[((int) (closeptr-last_comma))] = '\0';
                    
                    nesting--;
                }
                
                ParserElement parserElem = parseKVPair(keyval_pair, parser, lineNumber);
                if (HashMapContains((HashMap) value, parserElem->key)) {
                    fprintf(stderr, ERROR_STR "Element with key \"%s\" already exists in this table!\n", parser->filename, lineNumber, parserElem->key);
                    exit(EXIT_FAILURE);
                }
                HashMapPut((HashMap) value, parserElem->key, parserElem);

                if (oneLine) {
                    c = closeptr; // c advances past the subtable
                    last_element_lst = true;
                    free(keyval_pair);
                    continue;
                } else {
                    c = line+strlen(line); // c goes to the end of the line
                    continue;
                }
            }
            
            if (*c == '}') {
                nesting--;
                if (last_element_lst) {
                    last_element_lst = false;
                } else {
                    // Get the key value pair
                    char* keyval_pair = malloc(((int) (c-last_comma+1)) * sizeof(char)); assert(keyval_pair != NULL);
                    strncpy(keyval_pair, last_comma, ((int) (c-last_comma)));
                    keyval_pair[((int) (c-last_comma))] = '\0';

                    ParserElement parserElem = parseKVPair(keyval_pair, parser, lineNumber);
                    if (HashMapContains((HashMap) value, parserElem->key)) {
                        fprintf(stderr, ERROR_STR "Element with key \"%s\" already exists in this table!\n", parser->filename, lineNumber, parserElem->key);
                        exit(EXIT_FAILURE);
                    }

                    HashMapPut((HashMap) value, parserElem->key, parserElem);

                    free(keyval_pair);
                    last_comma = c+1;
                    
                    last_element_lst = true;
                    
                }
                if (nesting == 0) {
                    break;
                }
            }

            if (*c == '[') {    // List inside (must parse it first)
                char* keyval_pair = c;

                // Finds the ']' that correlates with the '[' from *c
                char* closeptr = c+1;
                int start_nesting = nesting++;
                while (start_nesting != nesting && *closeptr != '\0') {
                    if (*closeptr == '[') {
                        nesting++;
                    }
                    if (*closeptr == ']') {
                        nesting--;
                    }

                    closeptr++;
                }

                // The nesting itself should be 1 value larger (the last calculation sets nesting to start_nesting)
                nesting++;

                
                bool oneLine = closeptr != NULL;

                if (!oneLine) {
                    fprintf(stderr, ERROR_STR "Invalid value %s.\n", parser->filename, lineNumber, val);
                    exit(EXIT_FAILURE);
                }
                
                if (oneLine) {   // List ends this line (valstr is only the part from here to next line)
                    keyval_pair = malloc( ((int) (closeptr-last_comma+1)) * sizeof(char));
                    assert(keyval_pair != NULL);
                    strncpy(keyval_pair, last_comma, ((int) (closeptr-last_comma)));
                    keyval_pair[((int) (closeptr-last_comma))] = '\0';
                    
                    nesting--;
                }

                ParserElement parserElem = parseKVPair(keyval_pair, parser, lineNumber);
                if (HashMapContains((HashMap) value, parserElem->key)) {
                    fprintf(stderr, ERROR_STR "Element with key \"%s\" already exists in this table!\n", parser->filename, lineNumber, parserElem->key);
                    exit(EXIT_FAILURE);
                }
                HashMapPut((HashMap) value, parserElem->key, parserElem);

                if (oneLine) {
                    c = closeptr; // c advances past the sublist
                    last_element_lst = true;
                    free(keyval_pair);
                    continue;
                } else {
                    c = line+strlen(line); // c goes to the end of the line
                    continue;
                }
            }
            
            c++;
        }

    } else {
        fprintf(stderr, ERROR_STR "The value \"%s\" is not recognized.\n", parser->filename, lineNumber, val);
        exit(EXIT_FAILURE);
    }

    // TODO: change to not have hardcoded 50 max length
    char* ename = calloc(51, sizeof(char)); assert(ename != NULL);
    strncpy(ename, elem_name, 50);

    ParserElement elem = malloc(sizeof(struct parserelement));
    assert(elem != NULL);

    elem->key = ename;
    elem->value = value;
    elem->type = type;

    return elem;
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
    
    char* continuousVal = NULL; // This pointer stores a string that is continuously added on to for multiple line values (ex. lists)
    int continuousValSize = 0;
    char* continuousName = NULL; // This pointer stores a string that is the name associated to continuousVal

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

        // Handle comment (substitute that char by \0)
        int comment_idx = 0;
        int braces_nesting = 0; // {}
        int brackets_nesting = 0; // []
        int quotes_nesting = 0; // ""
        bool has_comment = false;
        while (line[comment_idx] != '\0') {
            char c = line[comment_idx];
            if (c == '[') {
                brackets_nesting++;
            }
            if (c == ']') {
                brackets_nesting--;
            }

            if (c == '{') {
                braces_nesting++;
            }
            if (c == '}') {
                braces_nesting--;
            }

            if (c == '"') {
                if (quotes_nesting == 0) {
                    quotes_nesting++;
                } else {
                    quotes_nesting--;
                }
            }

            if (c == '#' && braces_nesting == 0 && brackets_nesting == 0 && quotes_nesting == 0) {
                has_comment = true;
                break;
            }

            comment_idx++;
        }
        if (has_comment) {
            line[comment_idx] = '\0';
        }

        // If theres a previous value, then try to parse it more
        if (continuousVal != NULL) {
            char* trimmed_line = trim(line);
            continuousVal = realloc(continuousVal, sizeof(char)*(continuousValSize + strlen(trimmed_line)+1));
            assert(continuousVal != NULL);
            
            strncpy(continuousVal+continuousValSize, trimmed_line, strlen(trimmed_line));
            continuousValSize += strlen(trimmed_line);
            continuousVal[continuousValSize] = '\0';


            if (valueParsable(continuousVal)) {                
                ParserElement elem = parseValue(continuousName, continuousVal, parser, lineNumber);
                HashMapPut(currentTable->elements, elem->key, elem);
                free(continuousVal);
                free(continuousName);
                continuousVal = NULL;
                continuousValSize = 0;
                continuousName = NULL;
                continue;
            }
            // If the value if not parsable, the full line may be, so continue
        }

        char* trimmed_line = trim(line);
        char* table_name;
        int size = 0;
        if (*line == '[') { // May be new table
            char* c = trimmed_line+1;
            bool outside = false;
            while (*c != '\0') {
                if (isspace(*c)) {
                    fprintf(stderr, ERROR_STR "Invalid table name! (contains spaces)\n", parser->filename, lineNumber);
                    exit(EXIT_FAILURE);
                }
                if (*c == ']' && *(c+1) != '\0') {
                    fprintf(stderr, ERROR_STR "Invalid table name format. Must be [<name>]\n", parser->filename, lineNumber);
                    exit(EXIT_FAILURE);
                }

                if (*c == ']') {
                    size--;
                }

                c++;
                size++;
            }

            // Is new table
            table_name = malloc((size+1) * sizeof(char));
            assert(table_name != NULL);
            strncpy(table_name, trimmed_line+1, size);
            table_name[size] = '\0';
            ParserTable table = ParserTableCreate(table_name);
            assert(table != NULL);
            HashMapPut(res->tables, table_name, table);
            currentTable = table;
            continue;
        }

        
        int splitIdx = getSplitIdx(line);
        
        if (splitIdx <= 0) { // Also works if the first char is :
            if (continuousVal == NULL) { // Line is just wrong
                fprintf(stderr, ERROR_STR "Invalid line formatting. Must be <key> : <value>\n", parser->filename, lineNumber);
                exit(EXIT_FAILURE);
            } else {
                continue; // Line may not be wrong, so just skip next steps
            }
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

        if (*trimmed_val != '\0' && valueParsable(trimmed_val)) {
            if (continuousVal != NULL) {
                fprintf(stderr, ERROR_STR "The value \"%s\" is not recognized.\n", parser->filename, lineNumber, trimmed_val);
                exit(EXIT_FAILURE);
            }

            ParserElement elem = parseValue(trimmed_name, trimmed_val, parser, lineNumber);
            HashMapPut(currentTable->elements, elem->key, elem);
        } else {
            if (continuousVal == NULL) { // No previous string
                continuousVal = malloc(sizeof(char)*(strlen(trimmed_val)+1));
                assert(continuousVal != NULL);
                
                strncpy(continuousVal, trimmed_val, strlen(trimmed_val));
                continuousValSize = strlen(trimmed_val);
                continuousVal[continuousValSize] = '\0';
                
                continuousName = malloc(sizeof(char)*(strlen(trimmed_name)+1));
                assert(continuousName != NULL);
                strncpy(continuousName, trimmed_name, strlen(trimmed_name));
                continuousName[strlen(trimmed_name)] = '\0';
            } else {
                continuousVal = realloc(continuousVal, sizeof(char)*(continuousValSize + strlen(trimmed_val)+1));
                assert(continuousVal != NULL);
                
                strncpy(continuousVal+continuousValSize, trimmed_val, strlen(trimmed_val));
                continuousValSize += strlen(trimmed_val);
                continuousVal[continuousValSize] = '\0';
                
            }

            if (valueParsable(continuousVal)) {
                ParserElement elem = parseValue(continuousName, continuousVal, parser, lineNumber);
                HashMapPut(currentTable->elements, elem->key, elem);

                continuousVal = NULL;
                continuousValSize = 0;
                continuousName = NULL;
            }
        }
    }

    if (continuousVal != NULL) {
        free(continuousVal);
    }
    if (continuousName != NULL) {
        free(continuousName);
    }

    fclose(file);

    return parser->result;
}

ParserResult MapParserGetResult(MapParser parser) {
    assert(parser != NULL);

    return parser->result;
}


// Returns whether the parser table associated with tableName exists or not.
bool ParserResultHasTable(ParserResult res, char* tableName) {
    assert(res != NULL);
    assert(tableName != NULL);

    return HashMapContains(res->tables, tableName);
}

// Returns the parser table associated with tableName (NULL if invalid or unknown table name).
ParserTable ParserResultGetTable(ParserResult res, char* tableName) {
    assert(res != NULL);
    assert(tableName != NULL);

    if (!ParserResultHasTable(res, tableName)) {
        return NULL;
    }

    return (ParserTable) HashMapGet(res->tables, tableName);
}

// Returns the name associated with table.
char* ParserTableGetName(ParserTable table) {
    assert(table != NULL);

    return table->name;
}

// Returns whether the element associated with elementName exists or not.
bool ParserTableHasElement(ParserTable table, char* elementName) {
    assert(table != NULL);
    assert(elementName != NULL);

    return HashMapContains(table->elements, elementName);
}

// Returns the elemnt associated with elementName (NULL if invalid or unknown element name).
ParserElement ParserTableGetElement(ParserTable table, char* elementName) {
    assert(table != NULL);
    assert(elementName != NULL);

    if (!ParserTableHasElement(table, elementName)) {
        return NULL;
    }

    return (ParserElement) HashMapGet(table->elements, elementName);
}

// Returns a hashmap that associates element names to elements.
HashMap ParserTableGetHashMap(ParserTable table) {
    assert(table != NULL);

    return table->elements;
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
        
        // Must destroy the table and free the values
        case TABLE_TYPE:
            HashMap table = (HashMap) element->value;
            HashMapIterator iter = HashMapGetIterator(table);

            while (HashMapIterCanOperate(iter)) {
                ParserElement elem = (ParserElement) HashMapIterGetCurrentValue(iter);
                ParserElementDestroy(&elem);

                HashMapIterGoToNext(iter);
            }
            

            HashMapIterDestroy(&iter);
            HashMapDestroy(&table);
            break;
    }

    free(element);

    *elementp = NULL;
}

void ParserResultDestroy(ParserResult* resp) {
    assert(resp != NULL);
    assert(*resp != NULL);

    ParserResult res = *resp;

    HashMapIterator iter = HashMapGetIterator(res->tables);
    while (HashMapIterCanOperate(iter)) {
        ParserTable table = (ParserTable) HashMapIterGetCurrentValue(iter);

        ParserTableDestroy(&table);

        HashMapIterGoToNext(iter);
    }
    HashMapIterDestroy(&iter);
    HashMapDestroy(&(res->tables));

    free(res);

    *resp = NULL;
}
