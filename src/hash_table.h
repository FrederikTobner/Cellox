#ifndef CELLOX_TABLE_H_
#define CELLOX_TABLE_H_

#include "common.h"
#include "value.h"

/// @brief Type definition of an entry in a hashtable structure
typedef struct
{
    // Key of the entry 🔑
    object_string_t * key;
    // Value associated with the key
    value_t value;
} entry_t;

/// @brief Type definition of a hashtable structure
typedef struct
{
    // Number of entries in the hashtable
    uint32_t count;
    // Capacity of the hashtable
    uint32_t capacity;
    // Pointer to the first entry that is stored in the hashtable
    entry_t * entries;
} table_t;

/// @brief Dealocates the memory used by the hashtable
/// @param table The table where the contents are freed
void hash_table_free(table_t *table);

/// @brief Initializes the hashtable
/// @param table The hashtable that is initialized
void hash_table_init(table_t *table);

/// @brief Marks all the objects in the hashtable
/// @param table The hashtable where all the objects are marked
void hash_table_mark(table_t *table);

/// @brief Copys all the entries from a table to another table
/// @param from The source-table
/// @param to The destination-table
void hash_table_add_all(table_t * from, table_t * to);

/// @brief Attempts to Deletes an entry in the hashtable and returns true if an entry coresponding to the given key has been found
/// @param table The table where an attempt is made to delete an entry
/// @param key The key of the value that is deleted
/// @return A boolean value that indicates whether a value was deleted
bool hash_table_delete(table_t * table, object_string_t *key);

/// @brief Looks up a string in the hashtable
/// @param table The table where the string is searched
/// @param chars The underlying character representation of the string
/// @param length The length of the string
/// @param hash The hashvalue of the string
/// @return An object_string_t or NULL if the string hasn't been found
object_string_t * hash_table_find_string(table_t * table, char const * chars, uint32_t length, uint32_t hash);

// Reads the Value to the specified adress, if an entry corresponding to the given key is present and returns true if an entry coresponding to the given key has been found
bool hash_table_get(table_t * table, object_string_t * key, value_t * value);

/// @brief Removes the strings that are not referenced anymore from the table
/// @param table The table where all the values marked as white (not reachable) are removed
void hash_table_remove_white(table_t * table);

/// @brief Changes the value corresponding to the key or creates a new entry if no entry corespronding to the key has been found
/// @param table The table where the entry is changed or inserted
/// @param key The key of the entry that is changed or the  key of the new entry
/// @param value The value the value of the entry is changed to or value of the new entry
/// @return true if an entry coresponding to the given key has been found
bool hash_table_set(table_t * table, object_string_t * key, value_t value);

#endif
