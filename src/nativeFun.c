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
#include "nativeFun.h"
#include "object.h"
#include "value.h"

#define MAX_READ_LINE_INPUT 1024
#define MAX_USER_NAME_LENGTH 256

static void assertArity(char *functioName, int32_t arity, int32_t argcount);

Value clockNative(int32_t argCount, Value *args)
{
    assertArity("clock", 0, argCount);
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value exitNative(int32_t argCount, Value *args)
{
    assertArity("exit", 1, argCount);
    if (!IS_NUMBER(args[0]))
    {
        printf("exit can only be called with a number as argument");
        exit(65);
    }
    exit(AS_NUMBER(*args));
}

Value getUserNameNative(int32_t argCount, Value *args)
{
    assertArity("getUserName", 0, argCount);
#ifdef _WIN32
    DWORD bufCharCount = MAX_USER_NAME_LENGTH;
    TCHAR name[MAX_USER_NAME_LENGTH];
    GetUserNameA(name, &bufCharCount);
#endif
#ifdef linux
    char name[MAX_USER_NAME_LENGTH];
    getlogin_r(&name[0], MAX_USER_NAME_LENGTH);
#endif
    return OBJECT_VAL(copyString(&name[0], strlen(name), false));
}

Value randomNative(int32_t argCount, Value *args)
{
    assertArity("random", 0, argCount);
    return NUMBER_VAL(rand());
}

Value readLineNative(int32_t argCount, Value *args)
{
    assertArity("readLine", 0, argCount);
    char line[MAX_READ_LINE_INPUT];
    ObjectString result;
    fgets(line, sizeof(line), stdin);
    return OBJECT_VAL(copyString(line, strlen(line) - 1, false));
}

Value waitNative(int32_t argCount, Value *args)
{
    assertArity("wait", 1, argCount);
    if (!IS_NUMBER(args[0]))
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

static void assertArity(char *functioName, int32_t arity, int32_t argcount)
{
    if (arity != argcount)
    {
        printf("%s expects %d arguments but was called with %d", functioName, arity, argcount);
        exit(65);
    }
}
