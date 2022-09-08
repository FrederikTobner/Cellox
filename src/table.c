#include "table.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"

// The max load factor of the hashtable, if the max load factor multiplied with the capacity is reached is reached the hashtable grows
#define TABLE_MAX_LOAD 0.75

static void table_adjust_capacity(Table *, int32_t);
static Entry *table_find_entry(Entry *, int32_t, ObjectString *);

void table_free(Table *table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table_init(table);
}

void table_init(Table *table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void table_mark(Table *table)
{
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        memory_mark_object((Object *)entry->key);
        memory_mark_value(entry->value);
    }
}

void table_add_all(Table *from, Table *to)
{
    for (uint32_t i = 0; i < from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL)
            table_set(to, entry->key, entry->value);
    }
}

bool table_delete(Table *table, ObjectString *key)
{
    if (table->count == 0)
        return false;

    // Find the entry.
    Entry *entry = table_find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

ObjectString *table_find_string(Table *table, char const *chars, int32_t length, uint32_t hash)
{
    if (table->count == 0)
        return NULL;

    uint32_t index = hash % table->capacity;
    for (;;)
    {
        Entry *entry = &table->entries[index];
        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NULL(entry->value))
                return NULL;
        }
        else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length) == 0)
            return entry->key; // We found the string
        // We look in the next bucket but eventually we also have to wrap around the array when we reach the end
        index = (index + 1) % table->capacity;
    }
}

bool table_get(Table *table, ObjectString *key, Value *value)
{
    if (table->count == 0)
        return false;

    Entry *entry = table_find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    *value = entry->value;
    return true;
}

void table_remove_white(Table *table)
{
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked)
            table_delete(table, entry->key);
    }
}

bool table_set(Table *table, ObjectString *key, Value value)
{
    // We grow the hashtable when it becomes 75% full
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        uint32_t capacity = GROW_CAPACITY(table->capacity);
        table_adjust_capacity(table, capacity);
    }

    Entry *entry = table_find_entry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey && IS_NULL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

// Adjusts the capacity of the hash table
static void table_adjust_capacity(Table *table, int32_t capacity)
{
    Entry *entries = ALLOCATE(Entry, capacity);
    for (uint32_t i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }
    table->count = 0;
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key == NULL)
            continue;

        Entry *dest = table_find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

// Looks up an entry in the hashtable, based on a key
static Entry *table_find_entry(Entry *entries, int32_t capacity, ObjectString *key)
{
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;
    for (;;)
    {
        Entry *entry = &entries[index];
        if (entry->key == NULL)
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
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        else if (entry->key == key)
            return entry; // We found the key 🔑
        index = (index + 1) % capacity;
    }
}