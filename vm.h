#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

// The maximum amount of values that can be pushed onto the stack
#define STACK_MAX 256

// Type definition of a stackbased virtual machine
typedef struct
{
    Chunk *chunk;
    uint8_t *ip;
    // Stack based VM
    Value stack[STACK_MAX];
    Value *stackTop;
    Table globals;
    Table strings;
    Obj *objects;
} VM;

// Result of the interpretation (sucessfull, error during compilation or at runtime)
typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

// Creates a new VM
void initVM();

// Deallocates the memory used by the VM
void freeVM();

// Interprets a lox program
InterpretResult interpret(const char *source);

// Pushes a new Value on the stack
void push(Value value);

// Pops a value from the stack
Value pop();

#endif