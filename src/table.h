#ifndef cellox_table_h
#define cellox_table_h

#include "common.h"
#include "value.h"

// Type definition of an entry in a hashtable structure
typedef struct
{
    // Key of the entry 🔑
    object_string_t * key;
    // Value associated with the key
    value_t value;
} entry_t;

// Type definition of a hashtable structure
typedef struct
{
    // Number of entries in the hashtable
    uint32_t count;
    // Capacity of the hashtable
    uint32_t capacity;
    // Pointer to the first entry that is stored in the hashtable
    entry_t * entries;
} table_t;

// Dealocates the memory used by the hashtable
void table_free(table_t *table);

// Initializes the hashtable
void table_init(table_t *table);

// Marks all the objects in the hashtable
void table_mark(table_t *table);

// Copys all the entries from a table to another table
void table_add_all(table_t * from, table_t * to);

// Deletes an entry in the hashtable and returns true if an entry coresponding to the given key has been found
bool table_delete(table_t * table, object_string_t *key);

// Finds a string in the hashtable
object_string_t * table_find_string(table_t * table, char const * chars, uint32_t length, uint32_t hash);

// Reads the Value to the specified adress, if an entry corresponding to the given key is present and returns true if an entry coresponding to the given key has been found
bool table_get(table_t * table, object_string_t * key, value_t * value);

// Removes the strings that are not referenced anymore from the table
void table_remove_white(table_t * table);

// Changes the value corresponding to the key and returns true if an entry coresponding to the given key has been found
bool table_set(table_t * table, object_string_t * key, value_t value);

#endif