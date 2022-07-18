#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "table.h"

// Maximum amount of frames the virtual machine can hold
#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

// Type definition of a stackbased virtual machine
typedef struct
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    // Stack of the vm
    Value stack[STACK_MAX];
    /// Pointer to the top of the stack
    Value *stackTop;
    Table globals;
    Table strings;
    ObjUpvalue *openUpvalues;
    size_t bytesAllocated;
    size_t nextGC;
    Obj *objects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
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