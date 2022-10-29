#ifndef cellox_vm_h
#define cellox_vm_h

#include "object.h"
#include "table.h"

// Maximum amount of frames the virtual machine can hold - The maxiimum depth of the callstack
#define FRAMES_MAX 64

// Maximum amount values that can be allocated on the stack of the VirtualMachine - currently 16384 values
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

// type definition of a call frame structure - represents a single ongoing function call
typedef struct
{
    // The closure of the callframe
    object_closure_t * closure;
    // The instruction pointer in the callframe
    uint8_t * ip;
    // Points to the first slot in the stack of the virtualMachine the function can use
    value_t * slots;
} call_frame_t;

// Type definition of the stackbased virtual machine
typedef struct
{
    // Callstack of the virtual machine
    call_frame_t callStack[FRAMES_MAX];
    // The amount of callframes the virtualMachine currently holds
    uint32_t frameCount;
    // Amount of objects in the virtual machine that are marked as gray -> objects that are already discovered but haven't been processed yet
    uint32_t grayCount;
    // The capacity of the dynamic array storing the objects that were marked as gray
    uint32_t grayCapacity;
    // Stack of the virtualMachine
    value_t stack[STACK_MAX];
    /// Pointer to the top of the stack
    value_t * stackTop;
    // Hashtable that contains the global variables
    table_t globals;
    // Hashtable that contains the strings
    table_t strings;
    // String "init" used to look up the initializer of a class - reused for every init call
    object_string_t * initString;
    // Upvalues of the closures of all the functions on the callstack
    object_upvalue_t * openUpvalues;
    // Number of bytes that have been allocated by the virtualMachine
    size_t bytesAllocated;
    // A treshhold when the next garbage Collection shall be triggered (e.g. a Megabyte)
    size_t nextGC;
    // The objects that are allocated in the memory of the virtualMachine
    object_t * objects;
    // The stack that contains all the gray objects
    object_t ** grayStack;
} virtual_machine_t;

// Result of the interpretation (sucessfull, error during compilation or at runtime)
typedef enum
{
    // No error occured
    INTERPRET_OK,
    // Error that occured during the compilation process
    INTERPRET_COMPILE_ERROR,
    // Error that occured at runtime
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

extern virtual_machine_t virtualMachine;

// Deallocates the memory used by the VirtualMachine
void vm_free();

// Creates a new VirtualMachine
void vm_init();

// Interprets a lox program
interpret_result_t vm_interpret(char const *);

// Pushes a new Value on the stack
void vm_push(value_t);

// Pops a value from the stack
value_t vm_pop();

#endif