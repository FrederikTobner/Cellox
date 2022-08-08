#include "memory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "object.h"
#include "vm.h"

// Marko for allocating a new object
#define ALLOCATE_OBJECT(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)
// Offset basic for the fowler-noll-vo hash-fuction - 0x811c9dc5
#define OFFSET_BASIS 2166136261u

static Object *allocateObject(size_t, ObjectType);
static ObjectString *allocateString(char *, int32_t, uint32_t);
static uint32_t hashString(const char *, int32_t);
static void printFunction(ObjectFunction *);
static void removeFirstChar(char *next, int *length);
static void resolveEscapeSequence(char *next, int *length);

ObjectString *copyString(const char *chars, int32_t length)
{
    uint32_t hash = hashString(chars, length);
    ObjectString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned)
        return interned;
    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    char *next = NULL;
    bool containsEscapeSequences = false;
    while (next = strstr(heapChars, "\\"))
    {
        resolveEscapeSequence(next, &length);
        if (!containsEscapeSequences)
            containsEscapeSequences = true;
    }
    if (containsEscapeSequences)
    {
        // We have to look again for duplicates in the hashtable storing the strings allocated by the vm
        hash = hashString(heapChars, length);
        interned = tableFindString(&vm.strings, heapChars, length, hash);
        if (interned)
        {
            free(heapChars);
            return interned;
        }
        char *heapChars = ALLOCATE(char, length + 1);
    }
    return allocateString(heapChars, length, hash);
}

ObjectBoundMethod *newBoundMethod(Value receiver, ObjectClosure *method)
{
    ObjectBoundMethod *bound = ALLOCATE_OBJECT(ObjectBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjectClass *newClass(ObjectString *name)
{
    ObjectClass *celloxClass = ALLOCATE_OBJECT(ObjectClass, OBJ_CLASS);
    celloxClass->name = name;
    initTable(&celloxClass->methods);
    return celloxClass;
}

ObjectClosure *newClosure(ObjectFunction *function)
{
    ObjectUpvalue **upvalues = ALLOCATE(ObjectUpvalue *, function->upvalueCount);
    for (int32_t i = 0; i < function->upvalueCount; i++)
    {
        upvalues[i] = NULL;
    }
    ObjectClosure *closure = ALLOCATE_OBJECT(ObjectClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjectFunction *newFunction()
{
    ObjectFunction *function = ALLOCATE_OBJECT(ObjectFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjectInstance *newInstance(ObjectClass *celloxClass)
{
    ObjectInstance *instance = ALLOCATE_OBJECT(ObjectInstance, OBJ_INSTANCE);
    instance->celloxClass = celloxClass;
    initTable(&instance->fields);
    return instance;
}

ObjectNative *newNative(NativeFn function)
{
    ObjectNative *native = ALLOCATE_OBJECT(ObjectNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjectUpvalue *newUpvalue(Value *slot)
{
    // Allocating the memory used by the upvalue
    ObjectUpvalue *upvalue = ALLOCATE_OBJECT(ObjectUpvalue, OBJ_UPVALUE);
    // We zero out the closed field of the upvalue when we create it
    upvalue->closed = NULL_VAL;
    // Adress of the slot where the closed over variables live (enclosing environment)
    upvalue->location = slot;
    // When we allocate a new upvalue, it is not attached to any list
    upvalue->next = NULL;
    return upvalue;
}

void printObject(Value value)
{
    switch (OBJECT_TYPE(value))
    {
    case OBJ_BOUND_METHOD:
        printFunction(AS_BOUND_METHOD(value)->method->function);
        break;
    case OBJ_CLASS:
        printf("%s", AS_CLASS(value)->name->chars);
        break;
    case OBJ_CLOSURE:
        printFunction(AS_CLOSURE(value)->function);
        break;
    case OBJ_FUNCTION:
        printFunction(AS_FUNCTION(value));
        break;
    case OBJ_INSTANCE:
        printf("%s instance", AS_INSTANCE(value)->celloxClass->name->chars);
        break;
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    case OBJ_UPVALUE:
        printf("upvalue");
        break;
    }
}

ObjectString *takeString(char *chars, int32_t length)
{
    uint32_t hash = hashString(chars, length);
    ObjectString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(chars, length, hash);
}

// Allocates memory to store a string
static ObjectString *allocateString(char *chars, int32_t length, uint32_t hash)
{
    ObjectString *string = ALLOCATE_OBJECT(ObjectString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    push(OBJECT_VAL(string));
    // Adds the string to hashtable storing all the strings allocated by the vm
    tableSet(&vm.strings, string, NULL_VAL);
    pop();
    return string;
}

/*  FNV-1a hash function
 *   <href>https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function</href>
 */
static uint32_t hashString(const char *key, int32_t length)
{
    uint32_t hash = OFFSET_BASIS;
    for (int32_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

// Allocates the memory for an object of a given type
static Object *allocateObject(size_t size, ObjectType type)
{
    // Allocates the memory used by the Object
    Object *object = (Object *)reallocate(NULL, 0, size);
    // Sets the type of the object
    object->type = type;
    // Disables mark so it is picked up by the Garbage Collection in the next cycle
    object->isMarked = false;
    // Adds the object at the start of the linked list storing the objects allocated by the vm
    object->next = vm.objects;
    vm.objects = object;
#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void *)object, size, type);
#endif
    return object;
}

// Prints a function
static void printFunction(ObjectFunction *function)
{
    if (function->name == NULL)
    {
        // top level code
        printf("<script>");
        return;
    }
    // A function
    printf("<fn %s>", function->name->chars);
}

// Removes the first character in a sequence of characters
static void removeFirstChar(char *next, int *length)
{
    // Removes '\\' that precedes the escape sequence
    for (int j = 0; j < strlen(next); j++)
        next[j] = next[j + 1];
    next = '\0';
    (*length)--;
}

// Resolves all the escape sequences inside a string literal
static void resolveEscapeSequence(char *next, int *length)
{
    switch (*(next + 1))
    {
    // Alarm or beep
    case 'a':
        *(next + 1) = '\a';
        removeFirstChar(next, length);
        break;
    // Backspace
    case 'b':
        *(next + 1) = '\b';
        removeFirstChar(next, length);
        break;
    // New Line
    case 'n':
        *(next + 1) = '\n';
        removeFirstChar(next, length);
        break;
    // Carriage Return
    case 'r':
        *(next + 1) = '\r';
        removeFirstChar(next, length);
        break;
    // Tab Horizontal
    case 't':
        *(next + 1) = '\t';
        removeFirstChar(next, length);
        break;
    // Tab vertical
    case 'v':
        *(next + 1) = '\v';
        removeFirstChar(next, length);
        break;
    // Backslash, single and double quote
    case '\"':
        removeFirstChar(next, length);
        break;
    case '\'':
        removeFirstChar(next, length);
        break;
    case '\\':
        removeFirstChar(next, length);
        break;
    default:
        printf("Unknown escape sequence \\%c", *(next + 1));
        exit(65);
    }
}