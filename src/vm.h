#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "table.h"

// Maximum amount of frames the virtual machine can hold - The maxiimum depth of the callstack
#define FRAMES_MAX 64

// Maximum amount values that can be allocated on the stack of the VM - currently 16384 values
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

// type definition of a call frame structure - represents a single ongoing function call
typedef struct
{
    // The closure of the callframe
    ObjectClosure *closure;
    // The instruction pointer in the callframe
    uint8_t *ip;
    // Points to the first slot in the stack of the vm the function can use
    Value *slots;
} CallFrame;

// Type definition of a stackbased virtual machine
typedef struct
{
    // Callframes of the virtual machine
    CallFrame frames[FRAMES_MAX];
    // The amount of callframes the vm currently holds
    int32_t frameCount;
    // Amount of objects in the virtual machine that are marked as gray -> objects that are already discovered but haven't been processed yet
    int32_t grayCount;
    // The capacity of the dynamic array storing the objects that were marked as gray
    int32_t grayCapacity;
    // Stack of the vm
    Value stack[STACK_MAX];
    /// Pointer to the top of the stack
    Value *stackTop;
    // Hashtable that contains the global variables
    Table globals;
    // Hashtable that contains the strings
    Table strings;
    ObjectString *initString;
    ObjectUpvalue *openUpvalues;
    size_t bytesAllocated, // Number of bytes that have been allocated by the vm
        nextGC;            // A treshhold when the next garbage Collection shall be triggered (e.g. a Megabyte)
    // The objects that are allocated in the memory of the vm
    Object *objects;
    // The stack that coontains all the gray objects
    Object **grayStack;
} VM;

// Result of the interpretation (sucessfull, error during compilation or at runtime)
typedef enum
{
    // No error occured
    INTERPRET_OK,
    // Error during the compilation
    INTERPRET_COMPILE_ERROR,
    // Error at runtime
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

// Deallocates the memory used by the VM
void freeVM();

// Creates a new VM
void initVM();

// Interprets a lox program
InterpretResult interpret(const char *);

// Pushes a new Value on the stack
void push(Value);

// Pops a value from the stack
Value pop();

#endif