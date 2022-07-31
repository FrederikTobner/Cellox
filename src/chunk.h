#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// opcodes of the bytecode instruction set
typedef enum
{
    // Pops the two most upper values from the stack adds them and pushes the result onto the stack
    OP_ADD,
    /// Defines the arguments for the next function invocation
    OP_CALL,
    // Defines a new class
    OP_CLASS,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    // Defines a constant
    OP_CONSTANT,
    // Defines a global variable
    OP_DEFINE_GLOBAL,
    // Pops the two most upper values from the stack, divides the first with the second value and pushes the result on the stack
    OP_DIVIDE,
    // Determines whether two the values on top of the are equal and  pushes the result on the stack
    OP_EQUAL,
    // Pops the two most upper values from the stack, raises the first with the second value and pushes the result on the stack
    OP_EXPONENT,
    // Pushes the boolean value false on the stack
    OP_FALSE,
    // Gets the value of a global variable  and stores it on the stack
    OP_GET_GLOBAL,
    // Gets the value of a local variable and stores it on the stack
    OP_GET_LOCAL,
    // Gets the value of the property of a Cellox object and stores it on the stack
    OP_GET_PROPERTY,
    // Gets the parent class of a value and stores it on the stack
    OP_GET_SUPER,
    // Gets the value of the upvalue and stores it on the stack
    OP_GET_UPVALUE,
    // Pops the two most upper values from the stack, and pushes the value true on the stack if the first number is greater than the second number
    OP_GREATER,
    // Adds another class as the parent to a class declaration
    OP_INHERIT,
    // Invokes a function
    OP_INVOKE,
    // Jumps from the current position to another position in the code, determined by a certain offset
    OP_JUMP,
    // Jumps if the value on top of the stack is false
    OP_JUMP_IF_FALSE,
    // Pops the two most upper values from the stack, and pushes the value true on the stack if the first number is less than the second number
    OP_LESS,
    OP_LOOP,
    // Defines a new Method
    OP_METHOD,
    // Pops the two most upper values from the stack, divides the first with the second value and pushes the remainder of the division onto the stack
    OP_MODULO,
    // Pops the two most upper values from the stack adds them and pushes the result onto the stack
    OP_MULTIPLY,
    OP_NEGATE,
    OP_NOT,
    // Pushes a null value on the stack
    OP_NULL,
    // Pops a value from the stack and writes it to the standard output
    OP_PRINT,
    // Pops a value from the stack
    OP_POP,
    // Returns either a Value or null if nothing is specified
    OP_RETURN,
    OP_SET_LOCAL,
    OP_SET_GLOBAL,
    OP_SET_PROPERTY,
    OP_SET_UPVALUE,
    // Pops the two most upper values from the stack, subtracts the second value from the first value and pushes the result onto the stack
    OP_SUBTRACT,
    OP_SUPER_INVOKE,
    // Pushes the boolean value true on the stack
    OP_TRUE,
} Opcode;

/* Type definition of a dynamic array structure of bytecode instructions
bytecode instructions are idalized instructions for an abstract/virtual computer*/
typedef struct
{
    // Amount of bytecode instructions in the chunk
    int32_t count;
    // Capacity of the chunk
    int32_t capacity;
    // Operand Codes
    uint8_t *code;
    // STores line information to the corresponding lox program
    int32_t *lines;
    // Constants stored in the chunk
    ValueArray constants;
} Chunk;

// Adds a constant to the chunk
int32_t addConstant(Chunk *chunk, Value value);

// Free's a chunk (Deallocates the memory used by the chunk)
void freeChunk(Chunk *chunk);

// Initializes a chunk
void initChunk(Chunk *chunk);

// Write to a already existing chunk
void writeChunk(Chunk *chunk, uint8_t byte, int32_t line);

#endif