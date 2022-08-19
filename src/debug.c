#include "debug.h"

#include <stdio.h>

#include "object.h"
#include "value.h"

static int32_t byteInstruction(const char *, Chunk *, int32_t);
static int32_t constantInstruction(const char *, Chunk *, int32_t);
static int invokeInstruction(const char *, Chunk *, int);
static int32_t jumpInstruction(const char *, int32_t, Chunk *, int32_t);
static int32_t simpleInstruction(const char *, int32_t);

// Dissasembles a chunk of bytecode instructions
void disassembleChunk(Chunk *chunk, const char *name)
{
  printf("== %s ==\n", name);
  for (int32_t offset = 0; offset < chunk->count;)
  {
    offset = disassembleInstruction(chunk, offset);
  }
}

// Dissasembles a single instruction
int32_t disassembleInstruction(Chunk *chunk, int32_t offset)
{
  printf("%04X ", offset);
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    printf("   | ");
  else
    printf("%4d ", chunk->lines[offset]);
  // Instruction specific behaviour
  uint8_t instruction = chunk->code[offset];
  // Switch statement could be converted to a computed goto https://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables for efficiency
  printf(" OP_%02X: ", instruction);
  switch (instruction)
  {
  case OP_CONSTANT:
    return constantInstruction("CONSTANT", chunk, offset);
  case OP_NULL:
    return simpleInstruction("NULL", offset);
  case OP_GET_LOCAL:
    return byteInstruction("GET_LOCAL", chunk, offset);
  case OP_SET_LOCAL:
    return byteInstruction("SET_LOCAL", chunk, offset);
  case OP_GET_GLOBAL:
    return constantInstruction("GET_GLOBAL", chunk, offset);
  case OP_DEFINE_GLOBAL:
    return constantInstruction("DEFINE_GLOBAL", chunk, offset);
  case OP_SET_GLOBAL:
    return constantInstruction("SET_GLOBAL", chunk, offset);
  case OP_GET_UPVALUE:
    return byteInstruction("GET_UPVALUE", chunk, offset);
  case OP_SET_UPVALUE:
    return byteInstruction("SET_UPVALUE", chunk, offset);
  case OP_GET_PROPERTY:
    return constantInstruction("GET_PROPERTY", chunk, offset);
  case OP_SET_PROPERTY:
    return constantInstruction("SET_PROPERTY", chunk, offset);
  case OP_POP:
    return simpleInstruction("POP", offset);
  case OP_TRUE:
    return simpleInstruction("TRUE", offset);
  case OP_FALSE:
    return simpleInstruction("FALSE", offset);
  case OP_EQUAL:
    return simpleInstruction("EQUAL", offset);
  case OP_GREATER:
    return simpleInstruction("GREATER", offset);
  case OP_LESS:
    return simpleInstruction("LESS", offset);
  case OP_ADD:
    return simpleInstruction("ADD", offset);
  case OP_SUBTRACT:
    return simpleInstruction("SUBTRACT", offset);
  case OP_MULTIPLY:
    return simpleInstruction("MULTIPLY", offset);
  case OP_DIVIDE:
    return simpleInstruction("DIVIDE", offset);
  case OP_NOT:
    return simpleInstruction("NOT", offset);
  case OP_NEGATE:
    return simpleInstruction("NEGATE", offset);
  case OP_PRINT:
    return simpleInstruction("PRINT", offset);
  case OP_JUMP:
    return jumpInstruction("JUMP", 1, chunk, offset);
  case OP_JUMP_IF_FALSE:
    return jumpInstruction("JUMP_IF_FALSE", 1, chunk, offset);
  case OP_LOOP:
    return jumpInstruction("LOOP", -1, chunk, offset);
  case OP_CALL:
    return byteInstruction("CALL", chunk, offset);
  case OP_INVOKE:
    return invokeInstruction("INVOKE", chunk, offset);
  case OP_CLOSURE:
  {
    offset++;
    uint8_t constant = chunk->code[offset++];
    printf("%-16s %04X ", "CLOSURE", constant);
    printValue(chunk->constants.values[constant]);
    printf("\n");
    ObjectFunction *function = AS_FUNCTION(chunk->constants.values[constant]);
    for (int32_t j = 0; j < function->upvalueCount; j++)
    {
      int32_t isLocal = chunk->code[offset++];
      int32_t index = chunk->code[offset++];
      printf("%04X      |                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
    }
    return offset;
  }
  case OP_CLOSE_UPVALUE:
    return simpleInstruction("CLOSE_UPVALUE", offset);
  case OP_RETURN:
    return simpleInstruction("RETURN", offset);
  case OP_CLASS:
    return constantInstruction("CLASS", chunk, offset);
  case OP_METHOD:
    return constantInstruction("METHOD", chunk, offset);
  case OP_INHERIT:
    return simpleInstruction("INHERIT", offset);
  case OP_GET_SUPER:
    return constantInstruction("GET_SUPER", chunk, offset);
  case OP_SUPER_INVOKE:
    return invokeInstruction("SUPER_INVOKE", chunk, offset);
  case OP_MODULO:
    return simpleInstruction("MODULO", offset);
  case OP_EXPONENT:
    return simpleInstruction("EXPONENT", offset);
  default:
    printf("Unknown opcode %02X\n", instruction);
    return offset + 1;
  }
}

// Shows the slot number of a local variable
static int32_t byteInstruction(const char *name, Chunk *chunk, int32_t offset)
{
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %04X\n", name, slot);
  return offset + 2;
}

// Dissasembles a constant instruction - OP_CONSTANT
static int32_t constantInstruction(const char *name, Chunk *chunk, int32_t offset)
{
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %04X '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2;
}

// Dissasembles a invoke instruction
static int invokeInstruction(const char *name, Chunk *chunk, int offset)
{
  uint8_t constant = chunk->code[offset + 1];
  uint8_t argCount = chunk->code[offset + 2];
  printf("%-16s (%d args) %04X '", name, argCount, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 3;
}

// Dissasembles a jump instruction (with a 16-bit operand)
static int32_t jumpInstruction(const char *name, int32_t sign, Chunk *chunk, int32_t offset)
{
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %04X -> %04X\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

// Dissasembles a return instruction - OP_RETURN
static int32_t simpleInstruction(const char *name, int32_t offset)
{
  printf("%s\n", name);
  return offset + 1;
}