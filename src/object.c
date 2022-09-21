#include "memory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "object.h"
#include "string_utils.h"
#include "virtual_machine.h"

// Marko for allocating a new object
#define ALLOCATE_OBJECT(type, objectType) \
    (type *)object_allocate_object(sizeof(type), objectType)

// Offset basic for the fowler-noll-vo hash-fuction - 2166136261
#define OFFSET_BASIS 0x811c9dc5u

static Object *object_allocate_object(size_t, ObjectType);
static ObjectString *object_allocate_string(char *, uint32_t, uint32_t);
static uint32_t object_hash_string(char const *, uint32_t);
static void object_print_function(ObjectFunction *);

ObjectString *object_copy_string(char const *chars, uint32_t length, bool removeBackSlash)
{
    uint32_t hash;
    ObjectString *interned;
    char *heapChars;

    if (!string_utils_contains_character_restricted(chars, '\\', length))
    {
        hash = object_hash_string(chars, length);
        interned = table_find_string(&virtualMachine.strings, chars, length, hash);
        if (interned)
            return interned;
        heapChars = ALLOCATE(char, length + 1);
        memcpy(heapChars, chars, length);
        heapChars[length] = '\0';
    }
    else
    {
        heapChars = ALLOCATE(char, length + 1);
        memcpy(heapChars, chars, length);
        heapChars[length] = '\0';
        char *next = NULL;
        for (uint32_t i = 0; i < length; i++)
        {
            if(heapChars[i] == '\\')
                if(string_utils_resolve_escape_sequence(&heapChars[i], &length))
                return NULL;
        }
        // We have to look again for duplicates in the hashtable storing the strings allocated by the virtualMachine
        hash = object_hash_string(heapChars, length);
        interned = table_find_string(&virtualMachine.strings, heapChars, length, hash);
        if (interned)
        {
            free(heapChars);
            return interned;
        }
    }

    return object_allocate_string(heapChars, strlen(heapChars), hash);
}

ObjectBoundMethod *object_new_bound_method(Value receiver, ObjectClosure *method)
{
    ObjectBoundMethod *bound = ALLOCATE_OBJECT(ObjectBoundMethod, OBJECT_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjectClass *object_new_class(ObjectString *name)
{
    ObjectClass *celloxClass = ALLOCATE_OBJECT(ObjectClass, OBJECT_CLASS);
    celloxClass->name = name;
    table_init(&celloxClass->methods);
    return celloxClass;
}

ObjectClosure *object_new_closure(ObjectFunction *function)
{
    ObjectUpvalue **upvalues = ALLOCATE(ObjectUpvalue *, function->upvalueCount);
    for (uint32_t i = 0; i < function->upvalueCount; i++)
        upvalues[i] = NULL;
    ObjectClosure *closure = ALLOCATE_OBJECT(ObjectClosure, OBJECT_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjectFunction *object_new_function()
{
    ObjectFunction *function = ALLOCATE_OBJECT(ObjectFunction, OBJECT_FUNCTION);
    function->arity = 0u;
    function->upvalueCount = 0u;
    function->name = NULL;
    chunk_init(&function->chunk);
    return function;
}

ObjectInstance *object_new_instance(ObjectClass *celloxClass)
{
    ObjectInstance *instance = ALLOCATE_OBJECT(ObjectInstance, OBJECT_INSTANCE);
    instance->celloxClass = celloxClass;
    table_init(&instance->fields);
    return instance;
}

ObjectNative *object_new_native(NativeFn function)
{
    ObjectNative *native = ALLOCATE_OBJECT(ObjectNative, OBJECT_NATIVE);
    native->function = function;
    return native;
}

ObjectUpvalue *object_new_upvalue(Value *slot)
{
    // Allocating the memory used by the upvalue
    ObjectUpvalue *upvalue = ALLOCATE_OBJECT(ObjectUpvalue, OBJECT_UPVALUE);
    // We zero out the closed field of the upvalue when we create it
    upvalue->closed = NULL_VAL;
    // Adress of the slot where the closed over variables live (enclosing environment)
    upvalue->location = slot;
    // When we allocate a new upvalue, it is not attached to any list
    upvalue->next = NULL;
    return upvalue;
}

void object_print(Value value)
{
    switch (OBJECT_TYPE(value))
    {
    case OBJECT_BOUND_METHOD:
        object_print_function(AS_BOUND_METHOD(value)->method->function);
        break;
    case OBJECT_CLASS:
        printf("%s", AS_CLASS(value)->name->chars);
        break;
    case OBJECT_CLOSURE:
        object_print_function(AS_CLOSURE(value)->function);
        break;
    case OBJECT_FUNCTION:
        object_print_function(AS_FUNCTION(value));
        break;
    case OBJECT_INSTANCE:
        printf("%s instance", AS_INSTANCE(value)->celloxClass->name->chars);
        break;
    case OBJECT_NATIVE:
        printf("<native fn>");
        break;
    case OBJECT_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    case OBJECT_UPVALUE:
        printf("upvalue");
        break;
    }
}

ObjectString *object_take_string(char *chars, uint32_t length)
{
    uint32_t hash = object_hash_string(chars, length);
    ObjectString *interned = table_find_string(&virtualMachine.strings, chars, length, hash);
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return object_allocate_string(chars, length, hash);
}

// Allocates memory to store a string
static ObjectString *object_allocate_string(char *chars, uint32_t length, uint32_t hash)
{
    ObjectString *string = ALLOCATE_OBJECT(ObjectString, OBJECT_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    vm_push(OBJECT_VAL(string));
    // Adds the string to hashtable storing all the strings allocated by the virtualMachine
    table_set(&virtualMachine.strings, string, NULL_VAL);
    vm_pop();
    return string;
}

// Allocates the memory for an object of a given type
static Object *object_allocate_object(size_t size, ObjectType type)
{
    // Allocates the memory used by the Object
    Object *object = (Object *)memory_reallocate(NULL, 0, size);
    // Sets the type of the object
    object->type = type;
    // Disables mark so it is picked up by the Garbage Collection in the next cycle
    object->isMarked = false;
    // Adds the object at the start of the linked list storing the objects allocated by the virtualMachine
    object->next = virtualMachine.objects;
    virtualMachine.objects = object;
#ifdef DEBUG_LOG_GC
    printf("%p allocated %zu bytes for %d\n", (void *)object, size, type);
#endif
    return object;
}

/*  FNV-1a hash function
 *   <href>https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function</href>
 */
static uint32_t object_hash_string(char const *key, uint32_t length)
{
    uint32_t hash = OFFSET_BASIS;
    for (uint32_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 0x01000193u;
    }
    return hash;
}

// Prints a function
static void object_print_function(ObjectFunction *function)
{
    if (function->name == NULL)
    {
        // top level code
        printf("<script>");
        return;
    }
    // A function
    printf("<fun %s>", function->name->chars);
}
