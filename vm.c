#include <stdio.h>

#include "common.h"
#include "vm.h"
#include "debug.h"

VM vm;

static void resetStack()
{
    vm.stackTop = vm.stack;
}

void initVM()
{
    resetStack();
}

void freeVM()
{

}

static InterpretResult run()
{
    #define READ_BYTE() (*vm.ip++)

    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for(;;)
    {
        #ifdef DEBUG_TRACE_EXECUTION
            printf("          ");
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++)
            {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");
            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));        
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        //Declaration doesn't count as a statement in c -> therefore we execute an empty statement before the Declaration to avoid gettting an error during the compilation (Labels must be followed by a statement)
        case OP_CONSTANT: ;
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        
        case OP_RETURN:
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        
        default:
            return INTERPRET_COMPILE_ERROR;
        }
    }
    #undef READ_CONSTANT
    #undef READ_BYTE
}

InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}

void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}