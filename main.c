#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main (int argc, const char* argv[])
{
    //Initializes VM
    initVM();
    Chunk chunk;
    //Initialize chunk
    initChunk(&chunk);
    // Constant that is added to the chunk
    int constant = addConstant(&chunk, 1.2);
    //writes Constant operand code to the chunk
    writeChunk(&chunk, OP_CONSTANT, 11);
    //adds constant to the chunk
    writeChunk(&chunk, constant, 11);
    //Creates anotheer constant
    constant = addConstant(&chunk, 3.4);
    writeChunk(&chunk, OP_CONSTANT, 11);
    writeChunk(&chunk, constant, 11);
    writeChunk(&chunk, OP_ADD, 11);
    constant = addConstant(&chunk, 5.6);
    writeChunk(&chunk, OP_CONSTANT, 11);
    writeChunk(&chunk, constant, 11);
    writeChunk(&chunk, OP_DIVIDE, 11);
    //negates constant on top of the stack
    writeChunk(&chunk, OP_NEGATE, 11);
    //writes Return operand code to the chunk
    writeChunk(&chunk, OP_RETURN, 11);
    //Disassembles the chunk and prints it
    disassembleChunk(&chunk, "Dissasembled Chunk");
    //Interprets a chunk of Bytecode
    interpret(&chunk);
    //Deallocates the memory used by the virtual maschine
    freeVM();
    //Deallocates the memory used by the chunk
    freeChunk(&chunk);
    return 0;
}