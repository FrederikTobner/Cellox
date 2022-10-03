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
#include "native_fun.h"
#include "object.h"
#include "value.h"

#define MAX_READ_LINE_INPUT 1024
#define MAX_USER_NAME_LENGTH 256

static void native_assert_arrity(char const * functioName,  uint32_t arity, uint32_t argcount);

Value native_clock(uint32_t argCount, Value const * args)
{
    native_assert_arrity("clock", 0, argCount);
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value native_exit(uint32_t argCount, Value const * args)
{
    native_assert_arrity("exit", 1, argCount);
    if (!IS_NUMBER(*args))
    {
        printf("exit can only be called with a number as argument");
        exit(65);
    }
    exit(AS_NUMBER(*args));
}

Value native_get_username(uint32_t argCount, Value const * args)
{
    native_assert_arrity("getUserName", 0, argCount);
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

Value native_random(uint32_t argCount, Value const * args)
{
    native_assert_arrity("random", 0, argCount);
    return NUMBER_VAL(rand());
}

Value native_read_line(uint32_t argCount, Value const * args)
{
    native_assert_arrity("readLine", 0, argCount);
    char line[MAX_READ_LINE_INPUT];
    ObjectString result;
    fgets(line, sizeof(line), stdin);
    return OBJECT_VAL(object_copy_string(line, strlen(line) - 1, false));
}

Value native_string_length(uint32_t argCount, Value const * args)
{
    native_assert_arrity("strlen", 1, argCount);
    if(!IS_STRING(*args))
    {
        printf("strlen can only be called with a string as argument");
        exit(65);
    }
    return NUMBER_VAL(strlen(AS_CSTRING(*args)));
}

Value native_wait(uint32_t argCount, Value const * args)
{
    native_assert_arrity("wait", 1, argCount);
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

static void native_assert_arrity(char const * functioName, uint32_t arity, uint32_t argcount)
{
    if (arity != argcount)
    {
        printf("%s expects %d arguments but was called with %d", functioName, arity, argcount);
        exit(65);
    }
}
