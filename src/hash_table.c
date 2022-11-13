#include "hash_table.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"

// The max load factor of the hashtable, if the max load factor multiplied with the capacity is reached is reached the hashtable grows
#define TABLE_MAX_LOAD 0.75

static void hash_table_adjust_capacity(table_t * , int32_t );
static entry_t * hash_table_find_entry(entry_t * , int32_t , object_string_t *);

void hash_table_free(table_t * table)
{
    FREE_ARRAY(entry_t, table->entries, table->capacity);
    hash_table_init(table);
}

void hash_table_init(table_t * table)
{
    table->count = table->capacity = 0;
    table->entries = NULL;
}

void hash_table_mark(table_t * table)
{
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        entry_t * entry = table->entries + i;
        memory_mark_object((object_t *)entry->key);
        memory_mark_value(entry->value);
    }
}

void hash_table_add_all(table_t * from, table_t *to)
{
    for (uint32_t i = 0; i < from->capacity; i++)
    {
        entry_t * entry = from->entries + i;
        if (entry->key)
            hash_table_set(to, entry->key, entry->value);
    }
}

bool hash_table_delete(table_t *table, object_string_t *key)
{
    if (!table->count)
        return false;
    // Find the entry.
    entry_t * entry = hash_table_find_entry(table->entries, table->capacity, key);
    if (!entry->key)
        return false;
    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

object_string_t * hash_table_find_string(table_t * table, char const * chars, uint32_t length, uint32_t hash)
{
    if (!table->count)
        return NULL;

    uint32_t index = hash % table->capacity;
    for (;;)
    {
        entry_t * entry = &table->entries[index];
        if (!entry->key)
        {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NULL(entry->value))
                return NULL;
        }
        else if (entry->key->length == length && entry->key->hash == hash && !memcmp(entry->key->chars, chars, length))
            return entry->key; // We found the string
        // We look in the next bucket but eventually we also have to wrap around the array when we reach the end
        index = (index + 1) % table->capacity;
    }
}

bool hash_table_get(table_t * table, object_string_t * key, value_t * value)
{
    if (!table->count)
        return false;

    entry_t * entry = hash_table_find_entry(table->entries, table->capacity, key);
    if (!entry->key)
        return false;

    *value = entry->value;
    return true;
}

void hash_table_remove_white(table_t *table)
{
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        entry_t *entry = &table->entries[i];
        if (entry->key && !entry->key->obj.isMarked)
            hash_table_delete(table, entry->key);
    }
}

bool hash_table_set(table_t * table, object_string_t * key, value_t value)
{
    // We grow the hashtable when it becomes 75% full
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        uint32_t capacity = GROW_CAPACITY(table->capacity);
        hash_table_adjust_capacity(table, capacity);
    }

    entry_t *entry = hash_table_find_entry(table->entries, table->capacity, key);
    bool isNewKey = !entry->key;
    if (isNewKey && IS_NULL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

/// @brief Adjusts the capicity of a hashtable
/// @param table The hashtable where the capacity is changed
/// @param capacity The new capacity of the hashtable
static void hash_table_adjust_capacity(table_t * table, int32_t capacity)
{
    entry_t * entries = ALLOCATE(entry_t, capacity);
    for (uint32_t i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }
    table->count = 0;
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        entry_t * entry = table->entries + i;
        if (!entry->key)
            continue;
        entry_t * dest = hash_table_find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    FREE_ARRAY(entry_t, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

/// @brief Looks up an entry in the hashtable
/// @param entries The entries of the hashtable that is searched
/// @param capacity The capacity of the hashtable
/// @param key The key of the hashtable that is looked up
/// @return Return sthe entry or NULL if the value has already been deleted
static entry_t * hash_table_find_entry(entry_t * entries, int32_t capacity, object_string_t * key)
{
    uint32_t index = key->hash % capacity;
    entry_t *tombstone = NULL;
    for (;;)
    {
        entry_t *entry = &entries[index];
        if (!entry->key)
        {
            if (IS_NULL(entry->value))
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            else
            {
                /* We found a tombstone. 😱
                 * A tombstone marks the slot of a value that has already been deleted.
                 * This is done so because when we look up an entry that has been moved by a collision,
                 * the entry that has occupied the slot where the collision occured has been deleted,
                 * so we need a value to indicate that another value prevouisly occupied the slot so we don't stop looking for the entry*/
                if (!tombstone)
                    tombstone = entry;
            }
        }
        else if (entry->key == key)
            return entry; // We found the key 🔑
        index = (index + 1) % capacity;
    }
}
