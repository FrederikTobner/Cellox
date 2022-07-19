#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct
{
    ObjString *key;
    Value value;
} Entry;

typedef struct
{
    int_fast32_t count;
    int_fast32_t capacity;
    Entry *entries;
} Table;

// Initializes the hashtable
void initTable(Table *table);
// Dealocates the memory used by the hashtable
void freeTable(Table *table);
// Writes the Value to the specified adress, if an entry corresponding to the given key is present and returns true if an entry coresponding to the given key has been found
bool tableGet(Table *table, ObjString *key, Value *value);
// Changes the value corresponding to the key and returns true if an entry coresponding to the given key has been found
bool tableSet(Table *table, ObjString *key, Value value);
// Deletes an entry in the hashtable and returns true if an entry coresponding to the given key has been found
bool tableDelete(Table *table, ObjString *key);
// Copys all the entries from a table to another table
void tableAddAll(Table *from, Table *to);
// Finds a string in the hashtable
ObjString *tableFindString(Table *table, const char *chars, int_fast32_t length, uint_fast32_t hash);
// Removes the strings that are not referenced anymore from the table
void tableRemoveWhite(Table *table);
// Marks all the objects in the hashtable
void markTable(Table *table);

#endif