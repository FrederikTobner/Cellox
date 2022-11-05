#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef linux
#include <unistd.h>
#endif

#include "memory.h"
#include "native_functions.h"
#include "object.h"
#include "value.h"

typedef enum
{
    NATIVE_FUNCTION_CLASS_OF,
    NATIVE_FUNCTION_CLOCK,
    NATIVE_FUNCTION_EXIT,
    NATIVE_FUNCTION_GET_USER_NAME,
    NATIVE_FUNCTION_ON_LINUX,
    NATIVE_FUNCTION_ON_WINDOWS,
    NATIVE_FUNCTION_RANDOM,
    NATIVE_FUNCTION_READ_LINE,
    NATIVE_FUNCTION_STRLEN,
    NATIVE_FUNCTION_SYSTEM,
    NATIVE_FUNCTION_WAIT
}native_functions_t;

native_function_config_t native_function_configs [] = 
{
[NATIVE_FUNCTION_CLASS_OF]      =
{.functionName = "class_of", .function = native_classof, .arrity = 1},
[NATIVE_FUNCTION_CLOCK]         =
{.functionName = "clock", .function = native_clock},
[NATIVE_FUNCTION_EXIT]          =
{.functionName = "exit", .function = native_exit, .arrity = 1},
[NATIVE_FUNCTION_GET_USER_NAME] =
{.functionName = "get_user_name", .function = native_get_username},
[NATIVE_FUNCTION_ON_LINUX]      =
{.functionName = "on_linux", .function = native_on_linux},
[NATIVE_FUNCTION_ON_WINDOWS]    =
{.functionName = "on_windows", .function = native_on_windows},
[NATIVE_FUNCTION_RANDOM]        =
{.functionName = "random", .function = native_random},
[NATIVE_FUNCTION_READ_LINE]     =
{.functionName = "read_line", .function = native_read_line},
[NATIVE_FUNCTION_STRLEN]        =
{.functionName = "strlen", .function = native_string_length, .arrity = 1},
[NATIVE_FUNCTION_SYSTEM]        =
{.functionName = "system", .function = native_system, .arrity = 1},
[NATIVE_FUNCTION_WAIT]          =
{.functionName = "wait", .function = native_wait, .arrity = 1}
};

#define MAX_READ_LINE_INPUT 1024
#define MAX_USER_NAME_LENGTH 256

static void native_assert_arrity(uint8_t, uint32_t);

native_function_config_t * native_get_function_configs()
{
    return native_function_configs;
}

size_t native_get_function_count()
{
    return sizeof(native_function_configs) / sizeof(*native_function_configs);
}

value_t native_classof(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_CLASS_OF, argCount);
    if (!IS_INSTANCE(*args))
    {
        printf("class_of can only be called with an instance as argument");
        exit(65);
    }
    return OBJECT_VAL(AS_INSTANCE(*args)->celloxClass);
}

value_t native_clock(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_CLOCK, argCount);
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

value_t native_exit(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_EXIT, argCount);
    if (!IS_NUMBER(*args))
    {
        printf("exit can only be called with a number as argument");
        exit(65);
    }
    exit(AS_NUMBER(*args));
}

value_t native_get_username(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_GET_USER_NAME, argCount);
#ifdef _WIN32
    DWORD bufCharCount = MAX_USER_NAME_LENGTH;
    TCHAR name[MAX_USER_NAME_LENGTH];
    GetUserNameA(name, &bufCharCount);
#endif
#ifdef linux
    char name[MAX_USER_NAME_LENGTH];
    getlogin_r(&name[0], MAX_USER_NAME_LENGTH);
#endif
    return OBJECT_VAL(object_copy_string(&name[0], strlen(name), false));
}

value_t native_on_linux(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_ON_LINUX, argCount);
    #ifdef _WIN32
    return FALSE_VAL;
    #endif
    #ifdef linux
    return TRUE_VAL;
    #endif
}

value_t native_on_windows(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_ON_WINDOWS, argCount);
    #ifdef _WIN32
    return TRUE_VAL;
    #endif
    #ifdef linux
    return FALSE_VAL;
    #endif
}

value_t native_random(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_RANDOM, argCount);
    return NUMBER_VAL(rand());
}

value_t native_read_line(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_READ_LINE, argCount);
    char line[MAX_READ_LINE_INPUT];
    object_string_t result;
    fgets(line, sizeof(line), stdin);
    return OBJECT_VAL(object_copy_string(line, strlen(line) - 1, false));
}

value_t native_string_length(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_STRLEN, argCount);
    if(!IS_STRING(*args))
    {
        printf("strlen can only be called with a string as argument");
        exit(65);
    }
    return NUMBER_VAL(strlen(AS_CSTRING(*args)));
}

value_t native_system(uint32_t argCount, value_t const * args)
{
     native_assert_arrity(NATIVE_FUNCTION_SYSTEM, argCount);
    if (!IS_STRING(*args))
    {
        printf("system can only be called with a string as argument");
        exit(65);
    }
    system(AS_CSTRING(*args));
    return NULL_VAL;
}

value_t native_wait(uint32_t argCount, value_t const * args)
{
    native_assert_arrity(NATIVE_FUNCTION_WAIT, argCount);
    if (!IS_NUMBER(*args))
    {
        printf("wait can only be called with a number as argument");
        exit(65);
    }
#ifdef _WIN32
    // Milliseconds -> multiply with 1000
    Sleep(AS_NUMBER(*args) * 1000);
#endif
#ifdef linux
    // Seconds
    sleep(AS_NUMBER(*args));
#endif
    return NULL_VAL;
}

/// @brief Asserts that the native function was called with the appropriate argumentcount
/// @param function The native function that was called
/// @param argcount The amount of arguments that were used to call the native function
static void native_assert_arrity(uint8_t function, uint32_t argcount)
{
    if (native_function_configs[function].arrity != argcount)
    {
        printf("%s expects %zu arguments but was called with %d", 
        native_function_configs[function].functionName, 
        native_function_configs[function].arrity, 
        argcount);
        exit(65);
    }
}
