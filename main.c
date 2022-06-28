#include "common.h"
#include "chunk.h"
#include "debug.h"

int main (int argc, const char* argv[])
{
    Chunk chunk;
    initChunk(&chunk);
    int constant = addConstant(&chunk, 1234.56);
    writeChunk(&chunk, OP_CONSTANT, 11);
    writeChunk(&chunk, constant, 11);
    writeChunk(&chunk, OP_RETURN, 11);
    disassembleChunk(&chunk, "Dissasembled Chunk");
    freeChunk(&chunk);
    return 0;
}