/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file virtual_machine.c
 * @brief Header file containing of definitions of the virtual machine.
 * @details The virtual machine is stackbased.
 */

#include "virtual_machine.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "native_functions.h"
#include "value.h"

/// Global VirtualMachine variable
virtual_machine_t virtualMachine;

static void virtual_machine_array_literal(int32_t);
static bool virtual_machine_bind_method(object_class_t *, object_string_t *);
static bool virtual_machine_call(object_closure_t *, int32_t);
static bool virtual_machine_call_value(value_t, int32_t);
static object_upvalue_t * virtual_machine_capture_upvalue(value_t *);
static void virtual_machine_close_upvalues(value_t *);
static void virtual_machine_concatenate_arrays();
static void virtual_machine_concatenate_strings();
static void virtual_machine_define_method(object_string_t *);
static void virtual_machine_define_native(char const *, native_function_t);
static void virtual_machine_define_natives();
static bool virtual_machine_get_index_of();
static bool virtual_machine_get_sclice_of();
static bool virtual_machine_invoke(object_string_t *, int32_t);
static bool virtual_machine_invoke_from_class(object_class_t *, object_string_t *, int32_t);
static inline bool virtual_machine_is_falsey(value_t);
static bool virtual_machine_modulo();
static inline value_t virtual_machine_peek(int32_t);
static inline void virtual_machine_reset_stack();
static interpret_result virtual_machine_run();
static void virtual_machine_runtime_error(char const *, ...);
static bool virtual_machine_set_index_of();

void virtual_machine_free()
{
    hash_table_free(&virtualMachine.globals);
    hash_table_free(&virtualMachine.strings);
    virtualMachine.initString = NULL;
    if(virtualMachine.program)
        free(virtualMachine.program);
    memory_free_objects();
}

void virtual_machine_init()
{
    virtual_machine_reset_stack();
    virtualMachine.program = NULL;
    virtualMachine.objects = NULL;
    virtualMachine.bytesAllocated = 0;
    // Garbage Collection is triggered after 1 MB of data has been allocated
    virtualMachine.nextGC = (1 << 20);
    virtualMachine.grayCount = virtualMachine.grayCapacity = 0u;
    virtualMachine.grayStack = NULL;
    // Initializes the hashtable that contains the global variables
    hash_table_init(&virtualMachine.globals);
    // Initializes the hashtable that contains the strings
    hash_table_init(&virtualMachine.strings);
    //virtualMachine.stackTop = virtualMachine.stack;
    virtualMachine.initString = NULL;
    virtualMachine.initString = object_copy_string("init", 4u, false);
    // defines the native functions supported by the virtual machine
    virtual_machine_define_natives();
    // Sets seed value of the random number generator based on the time the vm was initialized
    srand(time(0));
}

interpret_result virtual_machine_interpret(char * program, bool freeProgram)
{
    object_function_t *function = compiler_compile(program);
    if (!function)
        return INTERPRET_COMPILE_ERROR;
    virtual_machine_push(OBJECT_VAL(function));
    object_closure_t *closure = object_new_closure(function);
    virtual_machine_pop();
    virtual_machine_push(OBJECT_VAL(closure));
    virtual_machine_call(closure, 0u);
    if(freeProgram)
        virtualMachine.program = program;
    return virtual_machine_run();
}

interpret_result virtual_machine_run_chunk(chunk_t chunk)
{
    object_function_t *function = object_new_function();
    function->chunk = chunk;
    function->upvalueCount = 0;
    function->arity = 0;
    if (!function)
        return INTERPRET_COMPILE_ERROR;
    virtual_machine_push(OBJECT_VAL(function));
    object_closure_t *closure = object_new_closure(function);
    virtual_machine_pop();
    virtual_machine_push(OBJECT_VAL(closure));
    virtual_machine_call(closure, 0u);
    return virtual_machine_run();
}

void virtual_machine_push(value_t value)
{
    // There are 16384 values on the stack 🤯
    if ((virtualMachine.stackTop - virtualMachine.stack) == STACK_MAX)
        virtual_machine_runtime_error("Stack overflow!!!");
    // We add the value to the stack
    *virtualMachine.stackTop = value;
    // The stacktop points at the next empty slot
    virtualMachine.stackTop++;
}

value_t virtual_machine_pop()
{
    // The stacktop points at the next empty slot -> we first decrement it
    virtualMachine.stackTop--;
    // Then we return the vakue stored at the stacktop
    return *virtualMachine.stackTop;
}

/// @brief Creates an array based on an array literal expression
/// @param argCount The size of the array
static void virtual_machine_array_literal(int32_t argCount)
{
    object_dynamic_value_array_t * dynamicArray = object_new_dynamic_value_array();
    value_t val;
    // The elements are reversed on the stack so we iterate backwards 🔙
    for (int32_t i = argCount - 1; i >= 0; i--)
        dynamic_value_array_write(&dynamicArray->array, virtual_machine_peek(i));
    for (int32_t j = 0; j < argCount; j++)
        virtual_machine_pop();
    virtual_machine_push(OBJECT_VAL(dynamicArray));
}

/// @brief Binds a method to a cellox class
/// @param celloxClass The class the method is bound to
/// @param name The name of the method
/// @return true if the method was defiened, false if not
static bool virtual_machine_bind_method(object_class_t * celloxClass, object_string_t * name)
{
    value_t method;
    if (!hash_table_get(&celloxClass->methods, name, &method))
    {
        virtual_machine_runtime_error("Undefined property '%s'.", name->chars);
        return false;
    }
    object_bound_method_t * bound = object_new_bound_method(virtual_machine_peek(0), AS_CLOSURE(method));
    virtual_machine_pop();
    virtual_machine_push(OBJECT_VAL(bound));
    return true;
}

/// @brief Calls a function that is bound to a closure
/// @param closure The closure the function belongs to
/// @param argCount The amount of arguments that are used when the function is envoked
/// @return true if everything went well, false if something went wrong (stack overflow / wrong argument count)
/// @note The function can fail if the function was called with a wrong amount of arguments or there are too much callframes on the callstack -> stack overflow
static bool virtual_machine_call(object_closure_t * closure, int32_t argCount) 
{
    if (argCount != closure->function->arity) 
    {
        virtual_machine_runtime_error("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (virtualMachine.frameCount == FRAMES_MAX) 
    {
        // The callstack is 64 calls deep 🤯
        virtual_machine_runtime_error("Stack overflow.");
        return false;
    }

    call_frame_t * frame = &virtualMachine.callStack[virtualMachine.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = virtualMachine.stackTop - argCount - 1;
    return true;
}

/// @brief Handles calls for anything that is not a function / closure
/// @param callee The value that is called
/// @param argCount The amount of arguments for the value call
/// @return true if everything went well, false if not
static bool virtual_machine_call_value(value_t callee, int32_t argCount)
{
    if (IS_OBJECT(callee)) 
    {
        switch (OBJECT_TYPE(callee)) 
        {
        case OBJECT_BOUND_METHOD: 
        {
            object_bound_method_t * bound = AS_BOUND_METHOD(callee);
            virtualMachine.stackTop[-argCount - 1] = bound->receiver;
            return virtual_machine_call(bound->method, argCount);
        }
        case OBJECT_CLASS: 
        {
            object_class_t * celloxClass = AS_CLASS(callee);
            virtualMachine.stackTop[-argCount - 1] = OBJECT_VAL(object_new_instance(celloxClass));
            value_t initializer;
            if (hash_table_get(&celloxClass->methods, virtualMachine.initString, &initializer))
                return virtual_machine_call(AS_CLOSURE(initializer), argCount);
            else if (argCount != 0) 
            {
                virtual_machine_runtime_error("Expected 0 arguments but got %d.", argCount);
                return false;
            }
            return true;
        }
        case OBJECT_CLOSURE:
            return virtual_machine_call(AS_CLOSURE(callee), argCount);
        case OBJECT_NATIVE: 
        {
            native_function_t native = AS_NATIVE(callee);
            value_t result = native(argCount, virtualMachine.stackTop - argCount);
            virtualMachine.stackTop -= argCount + 1;
            virtual_machine_push(result);
            return true;
        }
        default:
            virtual_machine_runtime_error("Can only call functions and classes, but call expression was performed with a %s object", 
                                    value_stringify_type(callee));
            return false;
        }
    }
     // Non-callable object type.
    virtual_machine_runtime_error("Can only call functions and classes, but call expression was performed with a %s value", 
                                    value_stringify_type(callee));
    return false;
}

/// @brief Captures an upvalue of the enclosing environment
/// @param local The slot of the local variable that is captured
/// @return The created upvalue
static object_upvalue_t * virtual_machine_capture_upvalue(value_t * local)
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
 * A upvalue is closed by copying the objects value into the closed field in te ObjectValue.
 */
static void virtual_machine_close_upvalues(value_t * last) 
{
    while (virtualMachine.openUpvalues && virtualMachine.openUpvalues->location >= last) 
    {
        object_upvalue_t * upvalue = virtualMachine.openUpvalues;
        upvalue->closed = * upvalue->location;
        upvalue->location = &upvalue->closed;
        virtualMachine.openUpvalues = upvalue->next;
    }
}

/// @brief Concatenates the two upper values (cellox arrays) on the stack
static void virtual_machine_concatenate_arrays()
{
    object_dynamic_value_array_t * newArray = object_new_dynamic_value_array();
    for (uint32_t i = 0; i < AS_ARRAY(virtual_machine_peek(1))->array.count; i++)
    {
        dynamic_value_array_write(&newArray->array, AS_ARRAY(virtual_machine_peek(1))->array.values[i]);
    }
    
    if(IS_ARRAY(virtual_machine_peek(0))) 
    {
        object_dynamic_value_array_t * array = AS_ARRAY(virtual_machine_peek(0));
        // Adding the same array twice results in an infinite loop
        uint32_t upperBound = array->array.count;
        for (uint32_t i = 0; i < upperBound; i++)
            dynamic_value_array_write(&newArray->array, array->array.values[i]);                    
    }                
    else
        dynamic_value_array_write(&newArray->array, virtual_machine_peek(0));
    virtual_machine_pop();
    virtual_machine_pop();
    virtual_machine_push(OBJECT_VAL(newArray));
}

/// @brief Concatenates the two upper values (cellox strings) on the stack
static void virtual_machine_concatenate_strings() 
{
    object_string_t * b = AS_STRING(virtual_machine_peek(0));
    object_string_t * a = AS_STRING(virtual_machine_peek(1));
    uint32_t length = a->length + b->length;
    char * chars = ALLOCATE(char, length + 1u);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';
    object_string_t * result = object_take_string(chars, length);
    virtual_machine_pop();
    virtual_machine_pop();
    virtual_machine_push(OBJECT_VAL(result));
}

/// @brief Defines a new Method in the hashTable of the cellox class instance
/// @param name The name of the method
static void virtual_machine_define_method(object_string_t * name) 
{
    value_t method = virtual_machine_peek(0);
    object_class_t *celloxClass = AS_CLASS(virtual_machine_peek(1));
    hash_table_set(&celloxClass->methods, name, method);
    virtual_machine_pop();
}

/// @brief Defines a native function for the virtual machine
/// @param name The name of the native function
/// @param function The function that is defined
static void virtual_machine_define_native(char const * name, native_function_t function) 
{
    virtual_machine_push(OBJECT_VAL(object_copy_string(name, (int32_t)strlen(name), false)));
    virtual_machine_push(OBJECT_VAL(object_new_native(function)));
    hash_table_set(&virtualMachine.globals, AS_STRING(virtualMachine.stack[0]), virtualMachine.stack[1]);
    virtual_machine_pop();
    virtual_machine_pop();
}

/// @brief Defines the native functions of the virtual machine
/// @details These are functions that are implemented in C
static void virtual_machine_define_natives()
{
    // Pointer to the first configuration
    native_function_config_t * configs = native_functions_get_function_configs();
    // Upper bound (pointer to the end of the memory segment where the native functions are stored)
    native_function_config_t * upperBound = configs + native_functions_get_function_count();
    for (native_function_config_t * nativeFunctionPointer = configs; nativeFunctionPointer < upperBound; nativeFunctionPointer++)
        virtual_machine_define_native(nativeFunctionPointer->functionName, nativeFunctionPointer->function);
}

/// @brief Gets an item in an array or a string specified by a numerical index
/// @return A boolean value that indicates whether the execution has led to a runtime error 
static bool virtual_machine_get_index_of()
{
    if (IS_NUMBER(virtual_machine_peek(0)) && IS_STRING(virtual_machine_peek(1)))
    {
        int num = AS_NUMBER(virtual_machine_pop());
        object_string_t * str = AS_STRING(virtual_machine_pop());
        if (num >= str->length || num < 0)
        {
            virtual_machine_runtime_error("accessed string out of bounds (at index %i)", num);
            return false;
        }
        char *chars = ALLOCATE(char, 2u);
        chars[0] = str->chars[num];
        chars[1] = '\0';
        object_string_t *result = object_take_string(chars, 1u);
        virtual_machine_push(OBJECT_VAL(result));
    }
    else if(IS_NUMBER(virtual_machine_peek(0)) && IS_ARRAY(virtual_machine_peek(1)))
    {
        int num = AS_NUMBER(virtual_machine_pop());
        object_dynamic_value_array_t * array = AS_ARRAY(virtual_machine_pop());
        if (num >= array->array.count || num < 0)
        {
            virtual_machine_runtime_error("accessed array out of bounds (at index %i)", num);
            return false;
        }
        virtual_machine_push(array->array.values[num]);
    }
    else
    {
        virtual_machine_runtime_error("Operands must a numerical value and a string object but are a %s %s and a %s %s", 
                                        value_stringify_type(virtual_machine_peek(0)),
                                        IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value", 
                                        value_stringify_type(virtual_machine_peek(1)),
                                        IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");
        return false;
    }
    return true;
}

/// @brief Creates a slice from an array
/// @return A boolean value that indicates whether the execution has led to a runtime error
/// @details Slices are subarrays of a soucearray
static bool virtual_machine_get_slice_of() 
{
    if(!IS_NUMBER(virtual_machine_peek(0)))
    {
        virtual_machine_runtime_error("A range can only be created with a number as first argument but was created with a %s %s", 
                                        value_stringify_type(virtual_machine_peek(0)),
                                        IS_OBJECT(virtual_machine_peek(2)) ? "object" : "value");
        return false;
    }
    if(!IS_NUMBER(virtual_machine_peek(1))) 
    {
        virtual_machine_runtime_error("A range can only be created with a number as second argument but was created with a %s %s", 
                                        value_stringify_type(virtual_machine_peek(1)),
                                        IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");
        return false;
    }
    if(!IS_ARRAY(virtual_machine_peek(2)) && !IS_STRING(virtual_machine_peek(2))) 
    {
        virtual_machine_runtime_error("A slice can only be created from an array but was tried to create with a %s %s", 
                                        value_stringify_type(virtual_machine_peek(2)),
                                        IS_OBJECT(virtual_machine_peek(2)) ? "object" : "value");
        return false;
    }
    
    int upperBound = AS_NUMBER(virtual_machine_pop());
    int i = AS_NUMBER(virtual_machine_pop());
    if(i >= upperBound) 
    {
        virtual_machine_runtime_error("Upper bound must be bigger than lower bound, but upper bound is %d and lower bound is %d", upperBound, i);
        return false;
    }
    if(i < 0) 
    {
        virtual_machine_runtime_error("LowerBound can not be negative, but is %d", i);
        return false;
    }
    if(IS_ARRAY(virtual_machine_peek(0)))
    {
        object_dynamic_value_array_t * sourceArray = AS_ARRAY(virtual_machine_pop());
        if(upperBound >= sourceArray->array.count) 
        {
            virtual_machine_runtime_error("Upperbound can not be higher or equal to the size of the array, but upperbound is %d and size %d", upperBound, sourceArray->array.count);
            return false;
        }
        object_dynamic_value_array_t * resultArray = object_new_dynamic_value_array();
        for (; i < upperBound; i++)
            dynamic_value_array_write(&resultArray->array, sourceArray->array.values[i]);
        virtual_machine_push(OBJECT_VAL(resultArray));
    }
    else 
    {
        object_string_t * sourceString = AS_STRING(virtual_machine_pop());
        if (upperBound >= sourceString->length) 
        {
            virtual_machine_runtime_error("Upperbound can not be higher or equal to the length of the string but upperbound is %d and size %d", upperBound, sourceString->length); 
            return false;
        }  
        char * chars = ALLOCATE(char, upperBound - i + 1);
        for (; i < upperBound; i++)
            chars[i] = sourceString->chars[i];
        chars[i] = '\0';
        object_string_t * resultSting = object_take_string(chars, upperBound - i + 1);
        virtual_machine_push(OBJECT_VAL(resultSting));
    }
    return true;
}

/// @brief Invokes a method bound to a cellox class instance
/// @param name The name of the method that is envoked
/// @param argCount The amount of arguments that are used when calling the method
/// @return true if everything went well, false if something went wrong (not a cellox instance / undefiened method / stack overflow / wrong argument count)
static bool virtual_machine_invoke(object_string_t * name, int32_t argCount) 
{
    value_t receiver = virtual_machine_peek(argCount);
    if (!IS_INSTANCE(receiver)) 
    {
        virtual_machine_runtime_error("Only instances have methods but a %s %s was invoked",
                                        value_stringify_type(receiver),
                                        IS_OBJECT(receiver) ? "object" : "value");
        return false;
    }
    object_instance_t *instance = AS_INSTANCE(receiver);
    value_t value;
    if (hash_table_get(&instance->fields, name, &value)) 
    {
        virtualMachine.stackTop[-argCount - 1] = value;
        return virtual_machine_call_value(value, argCount);
    }
    return virtual_machine_invoke_from_class(instance->celloxClass, name, argCount);
}

/// @brief Invokes a method from a celloxclass
/// @param celloxClass The class where the method is envoked
/// @param name Thee name of the method that is envoked
/// @param argCount The amount of arguments that are used when envoking the function
/// @return true if everything went well, false if something went wrong (undefiened method / stack overflow / wrong argument count)
static bool virtual_machine_invoke_from_class(object_class_t * celloxClass, object_string_t * name, int32_t argCount) 
{
    value_t method;
    if (!hash_table_get(&celloxClass->methods, name, &method)) {
        virtual_machine_runtime_error("Undefined property '%s'.", name->chars);
        return false;
    }
    return virtual_machine_call(AS_CLOSURE(method), argCount);
}

/// @brief  Determines if a value is falsey (either null or false)
/// @param value The value that is evalued
/// @return true if the value is null or false, otherwise false
static inline bool virtual_machine_is_falsey(value_t value) 
{
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

/// @brief Executes a modulo operation
/// @return A boolean value that indicates whether the execution has led to a runtime error 
static bool virtual_machine_modulo()
{
    if (IS_NUMBER(virtual_machine_peek(0)) && IS_NUMBER(virtual_machine_peek(1)))
    {
        int b = AS_NUMBER(virtual_machine_pop());
        int a = AS_NUMBER(virtual_machine_pop());
        virtual_machine_push(NUMBER_VAL(a % b));
    }
    else
    {
        virtual_machine_runtime_error("Operands must be two numbers but they are a %s %s and a %s %s", 
                                        value_stringify_type(virtual_machine_peek(0)),
                                        IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value",
                                        value_stringify_type(virtual_machine_peek(1)),
                                        IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");
        return false;
    }
    return true;
}

/// @brief Gets the value at the specified distance on the stack
/// @param distance The distance to the value
/// @return The value at the specified distance
static inline value_t virtual_machine_peek(int32_t distance) 
{
    return virtualMachine.stackTop[-1 - distance];
}

/// @brief Resets the stack of the vm
/// @details This means that all values will be removed
/// The upvalues and framecount is also reset. 
static inline void virtual_machine_reset_stack() 
{
    virtualMachine.stackTop = virtualMachine.stack;
    virtualMachine.frameCount = 0u;
    virtualMachine.openUpvalues = NULL;
}

static interpret_result virtual_machine_run() 
{
    #ifdef DEBUG_TRACE_EXECUTION
        printf("== execution ==\n");
    #endif

    /// Reads the next instruction from the current frame on top of the callstack
    #define READ_BYTE() \
    (*frame->ip++)

    /// Reads a single short (unsigned 16-bit integer value from the current frame on top of the callstack
    #define READ_SHORT() \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    /// Reads a constant from the closure of the current frame on the callstack
    #define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])

    /// Makro reads string in the chunk
    #define READ_STRING() AS_STRING(READ_CONSTANT())

    /**
    * Macro for creating a binary operator, based on a operator in C
    * We have to embed the marco into a do while, which isn't followed by a semicolon,
    * so all the statements in it get executed if they are after an if 🤮 
    */
    #define BINARY_OP(valueType, op)                                                                            \
    do                                                                                                          \
    {                                                                                                           \
        if (!IS_NUMBER(virtual_machine_peek(0)) || !IS_NUMBER(virtual_machine_peek(1)))                         \
        {                                                                                                       \
            virtual_machine_runtime_error("Operands must be numbers but they are a %s %s and a %s %s",          \
                                            value_stringify_type(virtual_machine_peek(0)),                      \
                                            IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value",            \
                                            value_stringify_type(virtual_machine_peek(1)),                      \
                                            IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");           \
            return INTERPRET_RUNTIME_ERROR;                                                                     \
        }                                                                                                       \
        double b = AS_NUMBER(virtual_machine_pop());                                                            \
        double a = AS_NUMBER(virtual_machine_pop());                                                            \
        virtual_machine_push(valueType(a op b));                                                                \
    } while (false)

    call_frame_t * frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];

    // For GCC and Clang we use computed goto's for non-debug builds, to create a efficient dispatch table, in order to speed up the execution.🚀
    // This is for example also done by ruby or dalvik (android java VM).
    // Lua on the other hand uses a regular switch statement like we do, if there is the compiler is not gcc or clang.
    #if !defined(BUILD_DEBUG) && (defined(COMPILER_GCC) || defined(COMPILER_Clang))

        // Dispatch table with the labels we jump to instead of function pointers
        void * dispatch_table [] = 
        {
            &&label_add, 
            &&label_array_literal, 
            &&label_call, 
            &&label_class,
            &&label_closure, 
            &&label_close_upvalue, 
            &&label_constant,
            &&label_define_global, 
            &&label_divide, 
            &&label_equal,
            &&label_exponent, 
            &&label_false, 
            &&label_get_global,
            &&label_get_index_of, 
            &&label_get_local, 
            &&label_get_property,
            &&label_get_slice_of, 
            &&label_get_super, 
            &&label_get_upvalue,
            &&label_greater, 
            &&label_inherit, 
            &&label_invoke, 
            &&label_jump,
            &&label_jump_if_false, 
            &&label_less, 
            &&label_loop, 
            &&label_method,
            &&label_modulo, 
            &&label_multiply, 
            &&label_negate, 
            &&label_not,
            &&label_null, 
            &&label_pop, 
            &&label_return ,
            &&label_set_global,
            &&label_set_index_of, 
            &&label_set_local, 
            &&label_set_property, 
            &&label_set_upvalue,
            &&label_subtract, 
            &&label_super_invoke,
            &&label_true
        };

        /// Makro that dipatches thenext bytecode instuction
        #define DISPATCH() goto *dispatch_table[READ_BYTE()]

        DISPATCH();
    #endif // Not debug and clang or gcc

    for (;;)
    {
        #ifdef DEBUG_TRACE_EXECUTION
            // Prints all the values located on the stack
            printf("          ");
            for (value_t * slot = virtualMachine.stack; slot < virtualMachine.stackTop; slot++)
            {
                printf("[ ");
                value_print(*slot);
                printf(" ]");
            }
            printf("\n");
            debug_disassemble_instruction(&frame->closure->function->chunk, (int32_t)(frame->ip - frame->closure->function->chunk.code));
        #endif
 
        #if !defined(BUILD_DEBUG) && (defined(COMPILER_GCC) || defined(COMPILER_Clang))
            label_add:
                if (IS_STRING(virtual_machine_peek(0)) && IS_STRING(virtual_machine_peek(1)))
                    virtual_machine_concatenate_strings();
                else if (IS_NUMBER(virtual_machine_peek(0)) && IS_NUMBER(virtual_machine_peek(1)))
                    BINARY_OP(NUMBER_VAL, +);
                else if(IS_ARRAY(virtual_machine_peek(1)))   
                    virtual_machine_concatenate_arrays();
                else
                {
                    virtual_machine_runtime_error("Operands must be two numbers, two strings, an array and a value or an array and an array, but they are a %s value and a %s value", 
                                                    value_stringify_type(virtual_machine_peek(0)), 
                                                    value_stringify_type(virtual_machine_peek(1)));
                    return INTERPRET_RUNTIME_ERROR;
                }
                DISPATCH();
            label_array_literal:
                virtual_machine_array_literal(READ_BYTE());
                DISPATCH();
            label_call:
            {
                int32_t argCount = READ_BYTE();
                if (!virtual_machine_call_value(virtual_machine_peek(argCount), argCount))
                    return INTERPRET_RUNTIME_ERROR;
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                DISPATCH();
            }
            label_closure:
            {
                object_function_t * function = AS_FUNCTION(READ_CONSTANT());
                object_closure_t * closure = object_new_closure(function);
                virtual_machine_push(OBJECT_VAL(closure));
                for (uint32_t i = 0; i < closure->upvalueCount; i++)
                {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    closure->upvalues[i] = isLocal ? virtual_machine_capture_upvalue(frame->slots + index) : frame->closure->upvalues[index];
                }
                DISPATCH();
            }
            label_class:
                virtual_machine_push(OBJECT_VAL(object_new_class(READ_STRING())));
                DISPATCH();
            label_close_upvalue:
                virtual_machine_close_upvalues(virtualMachine.stackTop - 1);
                virtual_machine_pop();
                DISPATCH();
            label_constant:
            {
                value_t constant = READ_CONSTANT();
                virtual_machine_push(constant);
                DISPATCH();
            }
            label_define_global:
            {
                object_string_t * name = READ_STRING();
                hash_table_set(&virtualMachine.globals, name, virtual_machine_peek(0));
                virtual_machine_pop();
                DISPATCH();
            }
            label_divide:
                BINARY_OP(NUMBER_VAL, /);
                DISPATCH();
            label_equal:
            {
                value_t a = virtual_machine_pop();
                value_t b = virtual_machine_pop();
                virtual_machine_push(BOOL_VAL(value_values_equal(a, b)));
                DISPATCH();
            }
            label_exponent:
                if (IS_NUMBER(virtual_machine_peek(0)) && IS_NUMBER(virtual_machine_peek(1)))
                {
                    double b = AS_NUMBER(virtual_machine_pop());
                    double a = AS_NUMBER(virtual_machine_pop());
                    virtual_machine_push(NUMBER_VAL(pow(a, b)));
                }
                else
                {
                    virtual_machine_runtime_error("Operands must be two numbers but they are a %s value and a %s value", 
                                                    value_stringify_type(virtual_machine_peek(0)), 
                                                    value_stringify_type(virtual_machine_peek(1)));
                    return INTERPRET_RUNTIME_ERROR;
                }
                DISPATCH();
            label_false:
                virtual_machine_push(BOOL_VAL(false));
                DISPATCH();
            label_get_global:
            {
                object_string_t * name = READ_STRING();
                value_t value;
                if (!hash_table_get(&virtualMachine.globals, name, &value))
                {
                    virtual_machine_runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                virtual_machine_push(value);
                DISPATCH();
            }
            label_get_index_of:
                if(!virtual_machine_get_index_of())
                    return INTERPRET_RUNTIME_ERROR; 
                DISPATCH();
            label_get_local:
                virtual_machine_push(frame->slots[READ_BYTE()]);
                DISPATCH();
            label_get_property:
            {
                if (!IS_INSTANCE(virtual_machine_peek(0)))
                {
                    virtual_machine_runtime_error("Only instances have properties but get expression but a %s %s was used", 
                                                    value_stringify_type(virtual_machine_peek(0)),
                                                    IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value");
                    return INTERPRET_RUNTIME_ERROR;
                }
                object_instance_t * instance = AS_INSTANCE(virtual_machine_peek(0));
                object_string_t * name = READ_STRING();
                value_t value;
                if (hash_table_get(&instance->fields, name, &value))
                {
                    virtual_machine_pop(); // Instance.
                    virtual_machine_push(value);
                    DISPATCH();
                }
                if (!virtual_machine_bind_method(instance->celloxClass, name))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                DISPATCH();
            }
            label_get_slice_of:
                if(!virtual_machine_get_slice_of())
                    return INTERPRET_RUNTIME_ERROR;
                DISPATCH();
            label_get_super:
            {
                object_string_t * name = READ_STRING();
                object_class_t * superclass = AS_CLASS(virtual_machine_pop());

                if (!virtual_machine_bind_method(superclass, name))
                {
                    virtual_machine_runtime_error("Method %s not defined in parent class %s", name->chars, superclass->name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                DISPATCH();
            }
            label_get_upvalue:
            {
                uint8_t slot = READ_BYTE();
                virtual_machine_push(*frame->closure->upvalues[slot]->location);
                DISPATCH();
            }
            label_greater:
                BINARY_OP(BOOL_VAL, >);
                DISPATCH();
            label_inherit:
            {
                value_t superclassvalue = virtual_machine_peek(1);
                if (!IS_CLASS(superclassvalue))
                {
                    virtual_machine_runtime_error("Superclass must be a class but is a %s %s",
                                                    value_stringify_type(superclassvalue),
                                                    IS_OBJECT(superclassvalue) ? "object" : "value");

                    return INTERPRET_RUNTIME_ERROR;
                }
                object_class_t * subclass = AS_CLASS(virtual_machine_peek(0));
                hash_table_add_all(&AS_CLASS(superclassvalue)->methods, &subclass->methods);
                virtual_machine_pop(); // Subclass.
                DISPATCH();
            }
            label_invoke:
            {
                object_string_t * method = READ_STRING();
                if (!virtual_machine_invoke(method, READ_BYTE()))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                DISPATCH();
            }
            label_jump:
                // We jump 🦘
                frame->ip += READ_SHORT();
                DISPATCH();
            label_jump_if_false:
            {
                uint16_t offset = READ_SHORT();
                if (virtual_machine_is_falsey(virtual_machine_peek(0)))
                    // We jump 🦘
                    frame->ip += offset;
                DISPATCH();
            }
            label_less:
                BINARY_OP(BOOL_VAL, <);
                DISPATCH();
            label_loop:
                frame->ip -= READ_SHORT();
                DISPATCH();
            label_method:
                virtual_machine_define_method(READ_STRING());
                DISPATCH();
            label_modulo:
                if (!virtual_machine_modulo())
                    return INTERPRET_RUNTIME_ERROR;
                DISPATCH();
            label_multiply:
                BINARY_OP(NUMBER_VAL, *);
                DISPATCH();
            label_negate:
                if (!IS_NUMBER(virtual_machine_peek(0)))
                {
                    virtual_machine_runtime_error("Operand must be a number but is a %s %s.", 
                                                    value_stringify_type(virtual_machine_peek(0)),
                                                    IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value");
                    return INTERPRET_RUNTIME_ERROR;
                }
                virtual_machine_push(NUMBER_VAL(-AS_NUMBER(virtual_machine_pop())));
                DISPATCH();
            label_not:
                virtual_machine_push(BOOL_VAL(virtual_machine_is_falsey(virtual_machine_pop())));
                DISPATCH();
            label_null:
                virtual_machine_push(NULL_VAL);
                DISPATCH();
            label_pop:
                virtual_machine_pop();
                DISPATCH();
            label_return:
            {
                value_t result = virtual_machine_pop();
                virtual_machine_close_upvalues(frame->slots);
                virtualMachine.frameCount--;
                if (!virtualMachine.frameCount)
                {
                    virtual_machine_pop();
                    return INTERPRET_OK;
                }
                virtualMachine.stackTop = frame->slots;
                virtual_machine_push(result);
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                DISPATCH();
            }
            label_set_global:
            {
                object_string_t * name = READ_STRING();
                if (hash_table_set(&virtualMachine.globals, name, virtual_machine_peek(0)))
                {
                    hash_table_delete(&virtualMachine.globals, name);
                    virtual_machine_runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                DISPATCH();
            }
            label_set_index_of:
                if (!virtual_machine_set_index_of()) 
                    return INTERPRET_RUNTIME_ERROR;
                DISPATCH();
            label_set_local:
                // We set the value at the specified slot to the value that is stored on the top of the stack of the virtual machine.
                frame->slots[READ_BYTE()] = virtual_machine_peek(0);
                DISPATCH();
            label_set_property:
            {
                if (!IS_INSTANCE(virtual_machine_peek(1)))
                {
                    virtual_machine_runtime_error("Only instances have fields but was called with a %s %s",
                                                    value_stringify_type(virtual_machine_peek(1)),
                                                    IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // We look up the field in the 'fields' hashtable of the cellox object instance
                hash_table_set(&AS_INSTANCE(virtual_machine_peek(1))->fields, READ_STRING(), virtual_machine_peek(0));
                // The value that is assigned to the property
                value_t value = virtual_machine_pop();
                virtual_machine_pop();
                virtual_machine_push(value);
                DISPATCH();
            }
            label_set_upvalue:
                *frame->closure->upvalues[READ_BYTE()]->location = virtual_machine_peek(0);
                DISPATCH();
            label_subtract:
                BINARY_OP(NUMBER_VAL, -);
                DISPATCH();
            label_super_invoke:
            {
                object_string_t * method = READ_STRING();
                int argCount = READ_BYTE();
                object_class_t * superclass = AS_CLASS(virtual_machine_pop());
                if (!virtual_machine_invoke_from_class(superclass, method, argCount))
                    return INTERPRET_RUNTIME_ERROR;
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                DISPATCH();
            }
            label_true:
                virtual_machine_push(BOOL_VAL(true));
                DISPATCH();
        // For MSVC and other C-compilers we use a switch statement instead
        #else
            uint8_t instruction;
            switch (instruction = READ_BYTE())
            {
            case OP_ADD:
            {
                if (IS_STRING(virtual_machine_peek(0)) && IS_STRING(virtual_machine_peek(1)))
                    virtual_machine_concatenate_strings();
                else if (IS_NUMBER(virtual_machine_peek(0)) && IS_NUMBER(virtual_machine_peek(1)))
                    BINARY_OP(NUMBER_VAL, +);
                else if(IS_ARRAY(virtual_machine_peek(1)))   
                    virtual_machine_concatenate_arrays();
                else
                {
                    virtual_machine_runtime_error("Operands must be two numbers, two strings, an array and a value or an array and an array, but they are a %s value and a %s value", 
                                                    value_stringify_type(virtual_machine_peek(0)), 
                                                    value_stringify_type(virtual_machine_peek(1)));
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_ARRAY_LITERAL:
            {
                virtual_machine_array_literal(READ_BYTE());
                break;
            }  
            case OP_CALL:
            {
                int32_t argCount = READ_BYTE();
                if (!virtual_machine_call_value(virtual_machine_peek(argCount), argCount))
                    return INTERPRET_RUNTIME_ERROR;
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                break;
            }
            case OP_CLOSURE:
            {
                object_function_t * function = AS_FUNCTION(READ_CONSTANT());
                object_closure_t * closure = object_new_closure(function);
                virtual_machine_push(OBJECT_VAL(closure));
                for (uint32_t i = 0; i < closure->upvalueCount; i++)
                {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    closure->upvalues[i] = isLocal ? virtual_machine_capture_upvalue(frame->slots + index) : frame->closure->upvalues[index];
                }
                break;
            }
            case OP_CLASS:
                virtual_machine_push(OBJECT_VAL(object_new_class(READ_STRING())));
                break;
            case OP_CLOSE_UPVALUE:
                virtual_machine_close_upvalues(virtualMachine.stackTop - 1);
                virtual_machine_pop();
                break;
            case OP_CONSTANT:
            {
                value_t constant = READ_CONSTANT();
                virtual_machine_push(constant);
                break;
            }
            case OP_DEFINE_GLOBAL:
            {
                object_string_t * name = READ_STRING();
                hash_table_set(&virtualMachine.globals, name, virtual_machine_peek(0));
                virtual_machine_pop();
                break;
            }
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;
            case OP_EQUAL:
            {
                value_t a = virtual_machine_pop();
                value_t b = virtual_machine_pop();
                virtual_machine_push(BOOL_VAL(value_values_equal(a, b)));
                break;
            }
            case OP_EXPONENT:
            {
                if (IS_NUMBER(virtual_machine_peek(0)) && IS_NUMBER(virtual_machine_peek(1)))
                {
                    double b = AS_NUMBER(virtual_machine_pop());
                    double a = AS_NUMBER(virtual_machine_pop());
                    virtual_machine_push(NUMBER_VAL(pow(a, b)));
                }
                else
                {
                    virtual_machine_runtime_error("Operands must be two numbers but they are a %s value and a %s value", 
                                                    value_stringify_type(virtual_machine_peek(0)), 
                                                    value_stringify_type(virtual_machine_peek(1)));
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_FALSE:
                virtual_machine_push(BOOL_VAL(false));
                break;
            case OP_GET_GLOBAL:
            {
                object_string_t * name = READ_STRING();
                value_t value;
                if (!hash_table_get(&virtualMachine.globals, name, &value))
                {
                    virtual_machine_runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                virtual_machine_push(value);
                break;
            }
            case OP_GET_INDEX_OF:
            {
                if(!virtual_machine_get_index_of())
                    return INTERPRET_RUNTIME_ERROR; 
                break;
            }
            case OP_GET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                virtual_machine_push(frame->slots[slot]);
                break;
            }
            case OP_GET_PROPERTY:
            {
                if (!IS_INSTANCE(virtual_machine_peek(0)))
                {
                    virtual_machine_runtime_error("Only instances have properties but get expression but a %s %s was used", 
                                                    value_stringify_type(virtual_machine_peek(0)),
                                                    IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value");
                    return INTERPRET_RUNTIME_ERROR;
                }
                object_instance_t * instance = AS_INSTANCE(virtual_machine_peek(0));
                object_string_t * name = READ_STRING();

                value_t value;
                if (hash_table_get(&instance->fields, name, &value))
                {
                    virtual_machine_pop(); // Instance.
                    virtual_machine_push(value);
                    break;
                }
                if (!virtual_machine_bind_method(instance->celloxClass, name))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_SLICE_OF:
            {
                if(!virtual_machine_get_slice_of())
                    return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_GET_SUPER:
            {
                object_string_t * name = READ_STRING();
                object_class_t * superclass = AS_CLASS(virtual_machine_pop());

                if (!virtual_machine_bind_method(superclass, name))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_UPVALUE:
            {
                uint8_t slot = READ_BYTE();
                virtual_machine_push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            case OP_INHERIT:
            {
                value_t superclass = virtual_machine_peek(1);
                if (!IS_CLASS(superclass))
                {
                    virtual_machine_runtime_error("Superclass must be a class but is a %s %s",
                                                    value_stringify_type(superclass),
                                                    IS_OBJECT(superclass) ? "object" : "value");

                    return INTERPRET_RUNTIME_ERROR;
                }
                object_class_t *subclass = AS_CLASS(virtual_machine_peek(0));
                hash_table_add_all(&AS_CLASS(superclass)->methods, &subclass->methods);
                virtual_machine_pop(); // Subclass.
                break;
            }
            case OP_INVOKE:
            {
                object_string_t * method = READ_STRING();
                int argCount = READ_BYTE();
                if (!virtual_machine_invoke(method, argCount))
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
                if (virtual_machine_is_falsey(virtual_machine_peek(0)))
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
                virtual_machine_define_method(READ_STRING());
                break;
            case OP_MODULO:
            {
                if (!virtual_machine_modulo())
                    return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(virtual_machine_peek(0)))
                {
                    virtual_machine_runtime_error("Operand must be a number but is a %s %s.", 
                                                    value_stringify_type(virtual_machine_peek(0)),
                                                    IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value");
                    return INTERPRET_RUNTIME_ERROR;
                }
                virtual_machine_push(NUMBER_VAL(-AS_NUMBER(virtual_machine_pop())));
                break;
            case OP_NOT:
                virtual_machine_push(BOOL_VAL(virtual_machine_is_falsey(virtual_machine_pop())));
                break;
            case OP_NULL:
                virtual_machine_push(NULL_VAL);
                break;
            case OP_POP:
                virtual_machine_pop();
                break;
            case OP_RETURN:
            {
                value_t result = virtual_machine_pop();
                virtual_machine_close_upvalues(frame->slots);
                virtualMachine.frameCount--;
                if (!virtualMachine.frameCount)
                {
                    virtual_machine_pop();
                    return INTERPRET_OK;
                }
                virtualMachine.stackTop = frame->slots;
                virtual_machine_push(result);
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                break;
            }
            case OP_SET_GLOBAL:
            {
                object_string_t * name = READ_STRING();
                if (hash_table_set(&virtualMachine.globals, name, virtual_machine_peek(0)))
                {
                    hash_table_delete(&virtualMachine.globals, name);
                    virtual_machine_runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_INDEX_OF:
            {
                if (!virtual_machine_set_index_of()) 
                    return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_SET_LOCAL:
            {
                // We set the value at the specified slot to the value that is stored on the top of the stack of the virtual machine.
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = virtual_machine_peek(0);
                break;
            }
            case OP_SET_PROPERTY:
            {
                if (!IS_INSTANCE(virtual_machine_peek(1)))
                {
                    virtual_machine_runtime_error("Only instances have fields but was called with a %s %s",
                                                    value_stringify_type(virtual_machine_peek(1)),
                                                    IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");
                    return INTERPRET_RUNTIME_ERROR;
                }
                object_instance_t * instance = AS_INSTANCE(virtual_machine_peek(1));
                hash_table_set(&instance->fields, READ_STRING(), virtual_machine_peek(0));
                value_t value = virtual_machine_pop();
                virtual_machine_pop();
                virtual_machine_push(value);
                break;
            }
            case OP_SET_UPVALUE:
            {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = virtual_machine_peek(0);
                break;
            }
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_SUPER_INVOKE:
            {
                object_string_t * method = READ_STRING();
                int argCount = READ_BYTE();
                object_class_t * superclass = AS_CLASS(virtual_machine_pop());
                if (!virtual_machine_invoke_from_class(superclass, method, argCount))
                    return INTERPRET_RUNTIME_ERROR;
                frame = &virtualMachine.callStack[virtualMachine.frameCount - 1];
                break;
            }
            case OP_TRUE:
                virtual_machine_push(BOOL_VAL(true));
                break;
            default:
            #if defined(COMPILER_MSVC) && !defined(BUILD_DEBUG)
                // We assume this code to be unreachable.
                // This tells the optimizer that reaching default is undefiened behaviour 😨
                __assume(0);
            #endif
                // When we debug we can take a slower but on the other hand safer approach
                printf("Bytecode instruction not supported by VM");
                exit(70);
            }
        #endif
    }
    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
    #ifdef COMPILER_GCC
        #undef DISPATCH
    #endif
}

/// @brief Reports an error that has occured at runtime
/// @param format The formater of the error message
/// @param ... Arguments that are passed in for the formatter
/// @details If no tests are executed the program exits with a exit code that indiactes an error at compile time
static void virtual_machine_runtime_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    for (int32_t i = virtualMachine.frameCount - 1; i >= 0; i--)
    {
        call_frame_t * frame = &virtualMachine.callStack[i];
        object_function_t *function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", chunk_determine_line_by_index(&function->chunk, instruction));
        if (!function->name)
            fprintf(stderr, "script\n");
        else
            fprintf(stderr, "%s()\n", function->name->chars);
    }
    virtual_machine_reset_stack();
}

/// @brief Executes a set index of operation
/// @return A boolean value that indicates whether the execution has led to a runtime error
static bool virtual_machine_set_index_of()
{
    if (IS_ARRAY(virtual_machine_peek(2)) && IS_NUMBER(virtual_machine_peek(1)))
    {
        value_t val = virtual_machine_pop();
        int num = AS_NUMBER(virtual_machine_pop());
        object_dynamic_value_array_t * array = AS_ARRAY(virtual_machine_pop());                
        if (num >= array->array.count || num < 0) 
        {
            virtual_machine_runtime_error("accessed array out of bounds at index %d", num);
            return false;
        }
        array->array.values[num] = val;
        virtual_machine_push(OBJECT_VAL(array));
    }
    else
    {
        virtual_machine_runtime_error("Can only be called with an used with an arry and a number but was used with a %s %s and a %s %s", 
                                        value_stringify_type(virtual_machine_peek(0)),
                                        IS_OBJECT(virtual_machine_peek(0)) ? "object" : "value", 
                                        value_stringify_type(virtual_machine_peek(1)),
                                        IS_OBJECT(virtual_machine_peek(1)) ? "object" : "value");
        return false;
    }
    return true;
}
