cmake_minimum_required(VERSION 3.16)

set(THIS CelloxTests)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

set(SOURCEPATH ../src/)

set(TESTSOURCES
assignmentoperator.cc
binaryoperator.cc
forLoops.cc
functions.cc
literalExpressions.cc
testUtils.cc
testUtils.hh
unaryoperator.cc
)

set(CELLOXSOURCES
${SOURCEPATH}chunk.c
${SOURCEPATH}compiler.c
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamicArray.c
${SOURCEPATH}debug.c
${SOURCEPATH}init.c 
${SOURCEPATH}lexer.c
${SOURCEPATH}memory.c 
${SOURCEPATH}nativeFun.c
${SOURCEPATH}object.c 
${SOURCEPATH}preProcessor.c
${SOURCEPATH}stringUtils.c
${SOURCEPATH}table.c
${SOURCEPATH}value.c  
${SOURCEPATH}vm.c
)

set(CELLOXHEADERS
${SOURCEPATH}chunk.h
${SOURCEPATH}common.h
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamicArray.h
${SOURCEPATH}debug.h
${SOURCEPATH}init.h
${SOURCEPATH}lexer.h
${SOURCEPATH}memory.h
${SOURCEPATH}nativeFun.h
${SOURCEPATH}object.h
${SOURCEPATH}preProcessor.h
${SOURCEPATH}stringUtils.h
${SOURCEPATH}table.h
${SOURCEPATH}value.h
${SOURCEPATH}vm.h
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(${THIS} ${TESTSOURCES} ${CELLOXSOURCES} ${CELLOXHEADERS})

target_include_directories(${THIS} PUBLIC ${SOURCEPATH})

target_link_libraries(
  ${THIS}
  GTest::gtest_main
)
include(googletest)

gtest_discover_tests(${THIS})