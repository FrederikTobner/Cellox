#ifndef cellox_table_h
#define cellox_table_h

#include "common.h"
#include "value.h"

// Type definition of an entry in a hashtable structure
typedef struct
{
    // Key of the entry 🔑
    ObjectString *key;
    // Value associated with the key
    Value value;
} Entry;

// Type definition of a hashtable structure
typedef struct
{
    // Number of entries in the hashtable
    uint32_t count;
    // Capacity of the hashtable
    uint32_t capacity;
    // Pointer to the first entry that is stored in the hashtable
    Entry * entries;
} Table;

// Dealocates the memory used by the hashtable
void table_free(Table *table);

// Initializes the hashtable
void table_init(Table *table);

// Marks all the objects in the hashtable
void table_mark(Table *table);

// Copys all the entries from a table to another table
void table_add_all(Table * from, Table * to);

// Deletes an entry in the hashtable and returns true if an entry coresponding to the given key has been found
bool table_delete(Table * table, ObjectString *key);

// Finds a string in the hashtable
ObjectString * table_find_string(Table * table, char const * chars, uint32_t length, uint32_t hash);

// Reads the Value to the specified adress, if an entry corresponding to the given key is present and returns true if an entry coresponding to the given key has been found
bool table_get(Table * table, ObjectString * key, Value * value);

// Removes the strings that are not referenced anymore from the table
void table_remove_white(Table * table);

// Changes the value corresponding to the key and returns true if an entry coresponding to the given key has been found
bool table_set(Table * table, ObjectString * key, Value value);

#endif