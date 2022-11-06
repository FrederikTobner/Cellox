#include "virtual_machine.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "native_functions.h"
#include "value.h"

/// @brief Global VirtualMachine variable
virtual_machine_t virtualMachine;

static bool vm_bind_method(object_class_t *, object_string_t *);
static bool vm_call(object_closure_t *, uint32_t);
static bool vm_call_value(value_t, uint32_t);
static object_upvalue_t *vm_capture_upvalue(value_t *);
static void vm_close_upvalues(value_t *);
static void vm_concatenate();
static void vm_define_method(object_string_t *);
static void vm_define_native(char const *, native_function_t);
static void vm_define_natives();
static bool vm_invoke(object_string_t *, uint32_t);
static bool vm_invoke_from_class(object_class_t *, object_string_t *, uint32_t);
static bool vm_is_falsey(value_t);
static value_t vm_peek(int32_t);
static void vm_reset_stack();
static interpret_result_t vm_run();
static void vm_runtime_error(char const *, ...);

void vm_free()
{
    table_free(&virtualMachine.globals);
    table_free(&virtualMachine.strings);
    virtualMachine.initString = NULL;
    if(virtualMachine.program)
        free(virtualMachine.program);
    memory_free_objects();
}

void vm_init()
{
    vm_reset_stack();
    virtualMachine.program = NULL;
    virtualMachine.objects = NULL;
    virtualMachine.bytesAllocated = 0;
    virtualMachine.nextGC = (1 << 20);
    virtualMachine.grayCount = virtualMachine.grayCapacity = 0u;
    virtualMachine.grayStack = NULL;
    // Initializes the hashtable that contains the global variables
    table_init(&virtualMachine.globals);
    // Initializes the hashtable that contains the strings
    table_init(&virtualMachine.strings);
    virtualMachine.initString = NULL;
    virtualMachine.initString = object_copy_string("init", 4u, false);
    // defines the native functions supported by the virtual machine
    vm_define_natives();
}

interpret_result_t vm_interpret(char * program, bool freeProgram)
{
    object_function_t *function = compiler_compile(program);
    if (function == NULL)
        return INTERPRET_COMPILE_ERROR;
    vm_push(OBJECT_VAL(function));
    object_closure_t *closure = object_new_closure(function);
    vm_pop();
    vm_push(OBJECT_VAL(closure));
    vm_call(closure, 0u);
    if(freeProgram)
        virtualMachine.program = program;
    return vm_run();
}

void vm_push(value_t value)
{
    // There are 16384 values on the stack 🤯
    if ((virtualMachine.stackTop - virtualMachine.stack) == STACK_MAX)
        vm_runtime_error("Stack overflow!!!");
    *virtualMachine.stackTop = value;
    virtualMachine.stackTop++;
}

value_t vm_pop()
{
    virtualMachine.stackTop--;
    return *virtualMachine.stackTop;
}

/// @brief Defines the native functions of the virtual machine
static void vm_define_natives()
{
    native_function_config_t * configs = native_get_function_configs();
    native_function_config_t * upperBound = configs + native_get_function_count();
    for (native_function_config_t * nativeFunctionPointer = configs; nativeFunctionPointer < upperBound; nativeFunctionPointer++)
        vm_define_native(nativeFunctionPointer->functionName, nativeFunctionPointer->function);
}

/// @brief Binds a method to a cellox class
/// @param celloxClass The class the method is bound to
/// @param name The name of the method
/// @return true if the method was defiened, false if not
static bool vm_bind_method(object_class_t * celloxClass, object_string_t * name)
{
    value_t method;
    if (!table_get(&celloxClass->methods, name, &method))
    {
        vm_runtime_error("Undefined property '%s'.", name->chars);
        return false;
    }
    object_bound_method_t *bound = object_new_bound_method(vm_peek(0), AS_CLOSURE(method));
    vm_pop();
    vm_push(OBJECT_VAL(bound));
    return true;
}

/// @brief Calls a function that is bound to a closure
/// @param closure The closure the function belongs to
/// @param argCount The amount of arguments that are used when the function is envoked
/// @return true if everything went well, false if something went wrong (stack overflow / wrong argument count)
static bool vm_call(object_closure_t * closure, uint32_t argCount)
{
    if (argCount != closure->function->arity)
    {
        vm_runtime_error("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (virtualMachine.frameCount == FRAMES_MAX)
    {
        // The callstack is 64 calls deep 🤯
        vm_runtime_error("Stack overflow.");
        return false;
    }

    call_frame_t * frame = &virtualMachine.callStack[virtualMachine.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = virtualMachine.stackTop - argCount - 1;
    return true;
}

static bool vm_call_value(value_t callee, uint32_t argCount)
{
    if (IS_OBJECT(callee))
    {
        switch (OBJECT_TYPE(callee))
        {
        case OBJECT_BOUND_METHOD:
        {
            object_bound_method_t * bound = AS_BOUND_METHOD(callee);
            virtualMachine.stackTop[-argCount - 1] = bound->receiver;
            return vm_call(bound->method, argCount);
        }
        case OBJECT_CLASS:
        {
            object_class_t * celloxClass = AS_CLASS(callee);
            virtualMachine.stackTop[-argCount - 1] = OBJECT_VAL(object_new_instance(celloxClass));
            value_t initializer;
            if (table_get(&celloxClass->methods, virtualMachine.initString, &initializer))
                return vm_call(AS_CLOSURE(initializer), argCount);
            else if (argCount != 0)
            {
                vm_runtime_error("Expected 0 arguments but got %d.", argCount);
                return false;
            }
            return true;
        }
        case OBJECT_CLOSURE:
            return vm_call(AS_CLOSURE(callee), argCount);
        case OBJECT_NATIVE:
        {
            native_function_t native = AS_NATIVE(callee);
            value_t result = native(argCount, virtualMachine.stackTop - argCount);
            virtualMachine.stackTop -= argCount + 1;
            vm_push(result);
            return true;
        }
        default:
            break; // Non-callable object type.
        }
    }
    vm_runtime_error("Can only call functions and classes.");
    return false;
}

/// @brief Captures an upvalue of the enclosing environment
/// @param local 
/// @return 
static object_upvalue_t * vm_capture_upvalue(value_t * local)
{
    object_upvalue_t * prevUpvalue = NULL;
    object_upvalue_t * upvalue = virtualMachine.openUpvalues;
    while (upvalue && upvalue->location > local)
    {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue && upvalue->location == local)
        return upvalue;
    object_upvalue_t * createdUpvalue = object_new_upvalue(local);
    createdUpvalue->next = upvalue;
    if (!prevUpvalue)
        virtualMachine.openUpvalues = createdUpvalue;
    else
        prevUpvalue->next = createdUpvalue;
    return createdUpvalue;
}

/** @brief Function takes a slot of the stack as a parameter.
 * @details Then it closes all upvalues it can find in that slot and the slots above that slot in the stack.
 * The concept of an upvalue is borrowed from Lua - see https://www.lua.org/pil/27.3.3.html.
 * A upvalue is closed by copying the objects value into the closed field in te ObjectValue.
 */
static void vm_close_upvalues(value_t * last)
{
    while (virtualMachine.openUpvalues && virtualMachine.openUpvalues->location >= last)
    {
        object_upvalue_t * upvalue = virtualMachine.openUpvalues;
        upvalue->closed = * upvalue->location;
        upvalue->location = &upvalue->closed;
        virtualMachine.openUpvalues = upvalue->next;
    }
}

/// @brief Concatenates the two upper values on the stack
static void vm_concatenate()
{
    object_string_t * b = AS_STRING(vm_peek(0));
    object_string_t * a = AS_STRING(vm_peek(1));
    uint32_t length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1u);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';
    object_string_t * result = object_take_string(chars, length);
    vm_pop();
    vm_pop();
    vm_push(OBJECT_VAL(result));
}

/// @brief Defines a new Method in the hashTable of the cellox class instance
/// @param name The name of the method
static void vm_define_method(object_string_t * name)
{
    value_t method = vm_peek(0);
    object_class_t *celloxClass = AS_CLASS(vm_peek(1));
    table_set(&celloxClass->methods, name, method);
    vm_pop();
}

/// @brief Defines a native function for the virtual machine
/// @param name The name of the native function
/// @param function The function that is defined
static void vm_define_native(char const * name, native_function_t function)
{
    vm_push(OBJECT_VAL(object_copy_string(name, (int32_t)strlen(name), false)));
    vm_push(OBJECT_VAL(object_new_native(function)));
    table_set(&virtualMachine.globals, AS_STRING(virtualMachine.stack[0]), virtualMachine.stack[1]);
    vm_pop();
    vm_pop();
}

/// @brief Invokes a method bound to a cellox class instance
/// @param name The name of the method that is envoked
/// @param argCount The amount of arguments that are used when calling the method
/// @return true if everything went well, false if something went wrong (not a cellox instance / undefiened method / stack overflow / wrong argument count)
static bool vm_invoke(object_string_t * name, uint32_t argCount)
{
    value_t receiver = vm_peek(argCount);
    if (!IS_INSTANCE(receiver))
    {
        vm_runtime_error("Only instances have methods.");
        return false;
    }
    object_instance_t *instance = AS_INSTANCE(receiver);
    value_t value;
    if (table_get(&instance->fields, name, &value))
    {
        virtualMachine.stackTop[-argCount - 1] = value;
        return vm_call_value(value, argCount);
    }
    return vm_invoke_from_class(instance->celloxClass, name, argCount);
}

/// @brief Invokes a method from a celloxclass
/// @param celloxClass The class where the method is envoked
/// @param name Thee name of the method that is envoked
/// @param argCount The amount of arguments that are used when envoking the function
/// @return true if everything went well, false if something went wrong (undefiened method / stack overflow / wrong argument count)
static bool vm_invoke_from_class(object_class_t * celloxClass, object_string_t * name, uint32_t argCount)
{
    value_t method;
    if (!table_get(&celloxClass->methods, name, &method))
    {
        vm_runtime_error("Undefined property '%s'.", name->chars);
        return false;
    }
    return vm_call(AS_CLOSURE(method), argCount);
}

/// @brief  Determines if a value is falsey (either null or false)
/// @param value The value that is evalued
/// @return true if the value is null or false, otherwise false
static bool vm_is_falsey(value_t value)
{
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

/// @brief Gets the value at the specified distance on the stacj
/// @param distance The distance to the value
/// @return The value at the specified distance
static value_t vm_peek(int32_t distance)
{
    return virtualMachine.stackTop[-1 - distance];
}

/// @brief Resets the stack of the vm
static void vm_reset_stack()
{
    virtualMachine.stackTop = virtualMachine.stack;
    virtualMachine.frameCount = 0u;
    virtualMachine.openUpvalues = NULL;
}

/// @brief Runs a lox program that was converted to bytecode instructions
/// @return OK if the program was executed sucessfull or a runtime error code if a runtime error occured
static interpret_result_t vm_run()
{
#ifdef DEBUG_TRACE_EXECUTION
    printf("== execution ==\n");
#endif

#define READ_BYTE() \
    (*frame->ip++)

#define READ_SHORT() \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])

// Makro readsa string in the chunk, at stores it as a constant
#define READ_STRING() AS_STRING(READ_CONSTANT())
/*Macro for creating a binary operator
We have to embed the marco into a do while, which isn't followed by a semicolon,
so all the statements in it get executed if they are after an if 🤮 */
#define BINARY_OP(valueType, op)                              \
    do                                                        \
    {                                                         \
        if (!IS_NUMBER(vm_peek(0)) || !IS_NUMBER(vm_peek(1))) \
        {                                                     \
            vm_runtime_error("Operands must be numbers but they are a %s value and a %s value", value_stringify_type(vm_peek(0)), value_stringify_type(vm_peek(1)));    \
            return INTERPRET_RUNTIME_ERROR;                   \
        }                                                     \
        double b = AS_NUMBER(vm_pop());                       \
        double a = AS_NUMBER(vm_pop());                       \
        vm_push(valueType(a op b));                           \
    } while (false)

    // Makro reads the next byte at the current positioon in the chunk
    call_frame_t *frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = virtualMachine.stack; slot < virtualMachine.stackTop; slot++)
        {
            printf("[ ");
            value_print(*slot);
            printf(" ]");
        }
        printf("\n");
        debug_disassemble_instruction(&frame->closure->function->chunk, (int32_t)(frame->ip - frame->closure->function->chunk.code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_ADD:
        {
            if (IS_STRING(vm_peek(0)) && IS_STRING(vm_peek(1)))
            {
                vm_concatenate();
            }
            else if (IS_NUMBER(vm_peek(0)) && IS_NUMBER(vm_peek(1)))
            {
                BINARY_OP(NUMBER_VAL, +);
            }
            else
            {
                vm_runtime_error("Operands must be two numbers or two strings but they are a %s value and a %s value", value_stringify_type(vm_peek(0)), value_stringify_type(vm_peek(1)));
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_CALL:
        {
            int32_t argCount = READ_BYTE();
            if (!vm_call_value(vm_peek(argCount), argCount))
            {
                // Amount of arguments used to call a function is not correct
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
            break;
        }
        case OP_CLOSURE:
        {
            object_function_t * function = AS_FUNCTION(READ_CONSTANT());
            object_closure_t * closure = object_new_closure(function);
            vm_push(OBJECT_VAL(closure));
            for (uint32_t i = 0; i < closure->upvalueCount; i++)
            {
                uint8_t isLocal = READ_BYTE();
                uint8_t index = READ_BYTE();
                closure->upvalues[i] = isLocal ? vm_capture_upvalue(frame->slots + index) : frame->closure->upvalues[index];
            }
            break;
        }
        case OP_CLASS:
            vm_push(OBJECT_VAL(object_new_class(READ_STRING())));
            break;
        case OP_CLOSE_UPVALUE:
            vm_close_upvalues(virtualMachine.stackTop - 1);
            vm_pop();
            break;
        case OP_CONSTANT:
        {
            value_t constant = READ_CONSTANT();
            vm_push(constant);
            break;
        }
        case OP_DEFINE_GLOBAL:
        {
            object_string_t * name = READ_STRING();
            table_set(&virtualMachine.globals, name, vm_peek(0));
            vm_pop();
            break;
        }
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;
        case OP_EQUAL:
        {
            value_t a = vm_pop();
            value_t b = vm_pop();
            vm_push(BOOL_VAL(value_values_equal(a, b)));
            break;
        }
        case OP_EXPONENT:
        {
            if (IS_NUMBER(vm_peek(0)) && IS_NUMBER(vm_peek(1)))
            {
                double b = AS_NUMBER(vm_pop());
                double a = AS_NUMBER(vm_pop());
                vm_push(NUMBER_VAL(pow(a, b)));
            }
            else
            {
                vm_runtime_error("Operands must be two numbers but they are a %s value and a %s value", value_stringify_type(vm_peek(0)), value_stringify_type(vm_peek(1)));
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_FALSE:
            vm_push(BOOL_VAL(false));
            break;
        case OP_GET_GLOBAL:
        {
            object_string_t * name = READ_STRING();
            value_t value;
            if (!table_get(&virtualMachine.globals, name, &value))
            {
                vm_runtime_error("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            vm_push(value);
            break;
        }
        case OP_GET_INDEX_OF:
        {
            if (IS_NUMBER(vm_peek(0)) && IS_STRING(vm_peek(1)))
            {
                int num = AS_NUMBER(vm_pop());
                object_string_t * str = AS_STRING(vm_pop());
                if (num >= str->length || num < 0)
                {
                    vm_runtime_error("accessed string out of bounds");
                    return INTERPRET_RUNTIME_ERROR;
                }
                char *chars = ALLOCATE(char, 2u);
                chars[0] = str->chars[num];
                chars[1] = '\0';
                object_string_t *result = object_take_string(chars, 1u);
                vm_push(OBJECT_VAL(result));
            }
            else
            {
                vm_runtime_error("Operands must a number and a string but was called with a %s value and a %s value", value_stringify_type(vm_peek(0)),  value_stringify_type(vm_peek(1)));
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_GET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            vm_push(frame->slots[slot]);
            break;
        }
        case OP_GET_PROPERTY:
        {
            if (!IS_INSTANCE(vm_peek(0)))
            {
                vm_runtime_error("Only instances have properties.");
                return INTERPRET_RUNTIME_ERROR;
            }
            object_instance_t * instance = AS_INSTANCE(vm_peek(0));
            object_string_t * name = READ_STRING();

            value_t value;
            if (table_get(&instance->fields, name, &value))
            {
                vm_pop(); // Instance.
                vm_push(value);
                break;
            }
            if (!vm_bind_method(instance->celloxClass, name))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
            vm_runtime_error("Undefined property '%s'.", name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
        case OP_GET_SUPER:
        {
            object_string_t * name = READ_STRING();
            object_class_t * superclass = AS_CLASS(vm_pop());

            if (!vm_bind_method(superclass, name))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_GET_UPVALUE:
        {
            uint8_t slot = READ_BYTE();
            vm_push(*frame->closure->upvalues[slot]->location);
            break;
        }
        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_INHERIT:
        {
            value_t superclass = vm_peek(1);
            if (!IS_CLASS(superclass))
            {
                vm_runtime_error("Superclass must be a class.");
                return INTERPRET_RUNTIME_ERROR;
            }
            object_class_t *subclass = AS_CLASS(vm_peek(0));
            table_add_all(&AS_CLASS(superclass)->methods, &subclass->methods);
            vm_pop(); // Subclass.
            break;
        }
        case OP_INVOKE:
        {
            object_string_t * method = READ_STRING();
            int argCount = READ_BYTE();
            if (!vm_invoke(method, argCount))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
            break;
        }
        case OP_JUMP:
        {
            uint16_t offset = READ_SHORT();
            // We jump 🦘
            frame->ip += offset;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            uint16_t offset = READ_SHORT();
            if (vm_is_falsey(vm_peek(0)))
                // We jump 🦘
                frame->ip += offset;
            break;
        }
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;
        case OP_LOOP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            break;
        }
        case OP_METHOD:
            vm_define_method(READ_STRING());
            break;
        case OP_MODULO:
        {
            if (IS_NUMBER(vm_peek(0)) && IS_NUMBER(vm_peek(1)))
            {
                int b = AS_NUMBER(vm_pop());
                int a = AS_NUMBER(vm_pop());
                vm_push(NUMBER_VAL(a % b));
            }
            else
            {
                vm_runtime_error("Operands must be two numbers but they are a %s value and a %s value", value_stringify_type(vm_peek(0)), value_stringify_type(vm_peek(1)));
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(vm_peek(0)))
            {
                vm_runtime_error("Operand must be a number but is a %s value.", value_stringify_type(vm_peek(0)));
                return INTERPRET_RUNTIME_ERROR;
            }
            vm_push(NUMBER_VAL(-AS_NUMBER(vm_pop(0))));
            break;
        case OP_NOT:
            vm_push(BOOL_VAL(vm_is_falsey(vm_pop())));
            break;
        case OP_NULL:
            vm_push(NULL_VAL);
            break;
        case OP_POP:
            vm_pop();
            break;
        case OP_PRINT:
            value_print(vm_pop());
            printf("\n");
            break;
        case OP_RETURN:
        {
            value_t result = vm_pop();
            vm_close_upvalues(frame->slots);
            virtualMachine.frameCount--;
            if (virtualMachine.frameCount == 0)
            {
                vm_pop();
                return INTERPRET_OK;
            }
            virtualMachine.stackTop = frame->slots;
            vm_push(result);
            frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
            break;
        }
        case OP_SET_GLOBAL:
        {
            object_string_t *name = READ_STRING();
            if (table_set(&virtualMachine.globals, name, vm_peek(0)))
            {
                table_delete(&virtualMachine.globals, name);
                vm_runtime_error("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SET_INDEX_OF:
        {
            if (IS_STRING(vm_peek(0)) && IS_NUMBER(vm_peek(1)) && IS_STRING(vm_peek(2)))
            {
                object_string_t * character = AS_STRING(vm_pop());
                int num = AS_NUMBER(vm_pop());
                object_string_t * str = AS_STRING(vm_pop());
                if (num >= str->length || num < 0 || character->length != 1)
                {
                    vm_runtime_error("accessed string out of bounds");
                    return INTERPRET_RUNTIME_ERROR;
                }
                char * newCharacterSequence = malloc(str->length + 1);
                memcpy(newCharacterSequence, str->chars, str->length); 
                newCharacterSequence[num] = character->chars[0];
                newCharacterSequence[str->length] = '\0';
                object_string_t * newString = object_take_string(newCharacterSequence, str->length);
                vm_push(OBJECT_VAL(newString));
            }
            else
            {
                vm_runtime_error("Can only be called with a sting a number and string but was called with a %s value and a %s value", value_stringify_type(vm_peek(0)),  value_stringify_type(vm_peek(1)));
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = vm_peek(0);
            break;
        }
        case OP_SET_PROPERTY:
        {
            if (!IS_INSTANCE(vm_peek(1)))
            {
                vm_runtime_error("Only instances have fields.");
                return INTERPRET_RUNTIME_ERROR;
            }
            object_instance_t * instance = AS_INSTANCE(vm_peek(1));
            table_set(&instance->fields, READ_STRING(), vm_peek(0));
            value_t value = vm_pop();
            vm_pop();
            vm_push(value);
            break;
        }
        case OP_SET_UPVALUE:
        {
            uint8_t slot = READ_BYTE();
            *frame->closure->upvalues[slot]->location = vm_peek(0);
            break;
        }
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_SUPER_INVOKE:
        {
            object_string_t * method = READ_STRING();
            int argCount = READ_BYTE();
            object_class_t * superclass = AS_CLASS(vm_pop());
            if (!vm_invoke_from_class(superclass, method, argCount))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
            break;
        }
        case OP_TRUE:
            vm_push(BOOL_VAL(true));
            break;
        default:
            printf("Bytecode intstruction not supported by VM");
        }
    }
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

/// @brief Reports an error that has occured at runtime
/// @param format The formater of the error message
/// @param args Arguments that are passed in for the formatter
static void vm_runtime_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    for (int32_t i = virtualMachine.frameCount - 1; i >= 0; i--)
    {
        call_frame_t * frame = &virtualMachine.callStack[i];
        object_function_t *function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (!function->name)
            fprintf(stderr, "script\n");
        else
            fprintf(stderr, "%s()\n", function->name->chars);
    }
    vm_reset_stack();
}
