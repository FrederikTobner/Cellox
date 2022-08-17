cmake_minimum_required(VERSION 3.16)

set(THIS CelloxTests)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

set(TESTSOURCES
binaryoperator.cc
assignmentoperator.cc
unaryoperator.cc
testUtils.cc
testUtils.hh
)

set(CELLOXSOURCES
../src/chunk.c
../src/compiler.c
../src/compiler.h
../src/dynamicArray.c
../src/debug.c
../src/init.c 
../src/lexer.c
../src/memory.c 
../src/nativeFun.c
../src/object.c 
../src/preProcessor.c
../src/stringUtils.c
../src/table.c
../src/value.c  
../src/vm.c
)

set(CELLOXHEADERS
../src/chunk.h
../src/common.h
../src/compiler.h
../src/dynamicArray.h
../src/debug.h
../src/init.h
../src/lexer.h
../src/memory.h
../src/nativeFun.h
../src/object.h
../src/preProcessor.h
../src/stringUtils.h
../src/table.h
../src/value.h
../src/vm.h
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(${THIS} ${TESTSOURCES} ${CELLOXSOURCES} ${CELLOXHEADERS})

target_include_directories(${THIS} PUBLIC ../src)

target_link_libraries(
  ${THIS}
  GTest::gtest_main
)
include(googletest)

gtest_discover_tests(${THIS})