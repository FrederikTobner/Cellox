#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#elif __unix__
#include <curses.h>
#include <unistd.h>
#endif

#include "memory.h"
#include "native_functions.h"
#include "object.h"
#include "value.h"
#include "virtual_machine.h"

typedef enum
{
    /// Native append_to_file function
    NATIVE_FUNCTION_APPEND_TO_FILE,
    /// Native class_of function
    NATIVE_FUNCTION_CLASS_OF,
    /// Native clock function
    NATIVE_FUNCTION_CLOCK,
    /// Native exit function
    NATIVE_FUNCTION_EXIT,
    /// Native on_linux function
    NATIVE_FUNCTION_ON_LINUX,
    /// Native on_macOS function
    NATIVE_FUNCTION_ON_MACOS,
    /// Native on_windows function
    NATIVE_FUNCTION_ON_WINDOWS,
    /// Native print function
    NATIVE_FUNCTION_PRINT,
    /// Native println function
    NATIVE_FUNCTION_PRINT_LINE,
    /// Native random function
    NATIVE_FUNCTION_RANDOM,
    /// Native read_file function
    NATIVE_FUNCTION_READ_FILE,
    /// Native read_key function
    NATIVE_FUNCTION_READ_KEY,
    /// Native read_line function
    NATIVE_FUNCTION_READ_LINE,
    /// Native strlen function
    NATIVE_FUNCTION_STRLEN,
    /// Native system function
    NATIVE_FUNCTION_SYSTEM,
    /// Native wait function
    NATIVE_FUNCTION_WAIT,
    /// Native write to file function
    NATIVE_FUNCTION_WRITE_TO_FILE
}native_functions_t;

native_function_config_t native_function_configs [] = 
{
    [NATIVE_FUNCTION_APPEND_TO_FILE]   =
    {
        .functionName = "append_to_file",
        .function = native_functions_append_to_file,
        .arrity = 2
    },
    [NATIVE_FUNCTION_CLASS_OF]          =
    {
        .functionName = "class_of", 
        .function = native_functions_classof, 
        .arrity = 1
    },
    [NATIVE_FUNCTION_CLOCK]             =
    {
        .functionName = "clock", 
        .function = native_functions_clock
    },
    [NATIVE_FUNCTION_EXIT]              =
    {
        .functionName = "exit", 
        .function = native_functions_exit, 
        .arrity = 1
    },
    [NATIVE_FUNCTION_ON_LINUX]          =
    {
        .functionName = "on_linux", 
        .function = native_functions_on_linux
    },
    [NATIVE_FUNCTION_ON_MACOS]          =
    {
        .functionName = "on_macOS", 
        .function = native_functions_on_macOS
    },
    [NATIVE_FUNCTION_ON_WINDOWS]        =
    {
        .functionName = "on_windows", 
        .function = native_functions_on_windows
    },
    [NATIVE_FUNCTION_PRINT]             =
    {
        .functionName = "print",
        .function = native_functions_print,
        .arrity = 1
    },
    [NATIVE_FUNCTION_PRINT_LINE]        =
    {
        .functionName = "println",
        .function = native_functions_print_line,
        .arrity = 1
    }, 
    [NATIVE_FUNCTION_RANDOM]            =
    {
        .functionName = "random", 
        .function = native_functions_random
    },
    [NATIVE_FUNCTION_READ_FILE]         =
    {
        .functionName = "read_file",
        .function = native_functions_read_file,
        .arrity = 1
    },
    [NATIVE_FUNCTION_READ_KEY]          = 
    {
        .functionName = "read_key", 
        .function = native_functions_read_key
    },
    [NATIVE_FUNCTION_READ_LINE]         =
    {
        .functionName = "read_line", 
        .function = native_functions_read_line
    },
    [NATIVE_FUNCTION_STRLEN]            =
    {
        .functionName = "strlen", 
        .function = native_functions_string_length, 
        .arrity = 1
    },
    [NATIVE_FUNCTION_SYSTEM]            =
    {
        .functionName = "system", 
        .function = native_functions_system, 
        .arrity = 1
    },
    [NATIVE_FUNCTION_WAIT]              =
    {
        .functionName = "wait", 
        .function = native_functions_wait, 
        .arrity = 1
    },
    [NATIVE_FUNCTION_WRITE_TO_FILE]    =
    {
        .functionName = "write_to_file",
        .function = native_functions_write_to_file,
        .arrity = 2
    }
};

#define MAX_READ_LINE_INPUT (1024)
#define MAX_USER_NAME_LENGTH (256)

static void native_functions_arguments_error(char const * format, ...);
static void native_functions_assert_arrity(uint8_t, uint32_t);

native_function_config_t * native_functions_get_function_configs()
{
    return native_function_configs;
}

size_t native_functions_get_function_count()
{
    return sizeof(native_function_configs) / sizeof(*native_function_configs);
}

value_t native_functions_append_to_file(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_APPEND_TO_FILE, argCount);
    if (!IS_STRING(*args) || !IS_STRING(*(args + 1)))
        native_functions_arguments_error("read_file can only be called with a string as argument");
    FILE * file;
    // Opens the file in append mode
    file = fopen(AS_CSTRING(*(args)), "a");
    if (!file)
        return FALSE_VAL;
    fprintf(file, "%s", AS_CSTRING(*(args + 1)));
    fclose(file);
    return TRUE_VAL;
}

value_t native_functions_classof(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_CLASS_OF, argCount);
    if (!IS_INSTANCE(*args))
        native_functions_arguments_error("class_of can only be called with an instance as argument");
    return OBJECT_VAL(AS_INSTANCE(*args)->celloxClass);
}

value_t native_functions_clock(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_CLOCK, argCount);
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

value_t native_functions_exit(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_EXIT, argCount);
    if (!IS_NUMBER(*args))
        native_functions_arguments_error("exit can only be called with a number as argument");
    int exitCode = AS_NUMBER(*args);
    virtual_machine_free();
    exit(exitCode);
}

value_t native_functions_on_linux(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_ON_LINUX, argCount);
    #ifdef linux
    return TRUE_VAL;
    #else
    return FALSE_VAL;
    #endif
}

value_t native_functions_on_macOS(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_ON_LINUX, argCount);
    #ifdef __APPLE__
    return TRUE_VAL;
    #else
    return FALSE_VAL;
    #endif
}

value_t native_functions_on_windows(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_ON_WINDOWS, argCount);
    #ifdef _WIN32
    return TRUE_VAL;
    #else
    return FALSE_VAL;
    #endif
   
}

value_t native_functions_print(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_PRINT, argCount);
    value_print(*args);
    return NULL_VAL;
}

value_t native_functions_print_line(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_PRINT_LINE, argCount);
    value_print(*args);
    putc('\n', stdout);
    return NULL_VAL;
}

value_t native_functions_random(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_RANDOM, argCount);
    return NUMBER_VAL(rand());
}

value_t native_functions_read_file(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_READ_FILE, argCount);
    if (!IS_STRING(*args))
        native_functions_arguments_error("read_file can only be called with a string as argument");
    char * path = AS_CSTRING(*args);
    // Open file in read-only mode
    FILE * file = fopen(path, "r");
    if (!file)
        return NULL_VAL;
    // Seek end of the file
    fseek(file, 0L, SEEK_END);
    // Store filesize
    size_t fileSize = ftell(file);
    // Rewind filepointer to the beginning of the file
    rewind(file);
    // Allocate memory apropriate to store the file
    char * buffer = (char *)malloc(fileSize);
    if (!buffer)
        return NULL_VAL;
    // Store amount of read bytes
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
        return NULL_VAL;
    fclose(file);
    // Create cellox string from content stored in the character buffer
    return OBJECT_VAL(object_copy_string(buffer, fileSize, false));
}

value_t native_functions_read_key(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_READ_KEY, argCount);
    char character = getchar();
    object_string_t result;
    return OBJECT_VAL(object_copy_string(&character, 1, false));
}

value_t native_functions_read_line(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_READ_LINE, argCount);
    char line[MAX_READ_LINE_INPUT];
    object_string_t result;
    fgets(line, sizeof(line), stdin);
    return OBJECT_VAL(object_copy_string(line, strlen(line) - 1, false));
}

value_t native_functions_string_length(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_STRLEN, argCount);
    if(!IS_STRING(*args))
        native_functions_arguments_error("strlen can only be called with a string as argument but was called with %s", value_stringify_type(*args));
    return NUMBER_VAL(strlen(AS_CSTRING(*args)));
}

value_t native_functions_system(uint32_t argCount, value_t const * args)
{
     native_functions_assert_arrity(NATIVE_FUNCTION_SYSTEM, argCount);
    if (!IS_STRING(*args))
        native_functions_arguments_error("system can only be called with a string as argument");
    system(AS_CSTRING(*args));
    return NULL_VAL;
}

value_t native_functions_wait(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_WAIT, argCount);
    if (!IS_NUMBER(*args))
        native_functions_arguments_error("wait can only be called with a number as argument");
    #ifdef _WIN32
    // Milliseconds -> multiply with 1000
    Sleep(AS_NUMBER(*args) * 1000);
    #elif __unix__
    // Seconds
    sleep(AS_NUMBER(*args));
    #endif
    return NULL_VAL;
}

value_t native_functions_write_to_file(uint32_t argCount, value_t const * args)
{
    native_functions_assert_arrity(NATIVE_FUNCTION_WRITE_TO_FILE, argCount);
    if (!IS_STRING(*args) || !IS_STRING(*(args + 1)))
        native_functions_arguments_error("read_file can only be called with a string as argument");
    FILE * file;
    // Open file in write mode
    file = fopen(AS_CSTRING(*(args)), "w");
    if (!file)
        return FALSE_VAL; // File could not be openend -> false indicates a failure
    fprintf(file, "%s", AS_CSTRING(*(args + 1)));
    fclose(file);
    // True indicates a successfull exection
    return TRUE_VAL;
}

/// @brief Asserts that the native function was called with the appropriate argumentcount
/// @param function The native function that was called
/// @param argcount The amount of arguments that were used to call the native function
static void native_functions_assert_arrity(uint8_t function, uint32_t argcount)
{
    if (native_function_configs[function].arrity != argcount)
    {
        native_functions_arguments_error("%s expects %zu arguments but was called with %d", 
        native_function_configs[function].functionName, 
        native_function_configs[function].arrity, 
        argcount);
    }
}

/// @brief Emits a error message regarding a faulty native function call and exits with the appropriate exit code (70 - runtime error) 
/// @param format The format of the error message
/// @param ... The arguments that are printed using the format
static void native_functions_arguments_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    virtual_machine_free();
    exit(EXIT_CODE_COMPILATION_ERROR);
}
