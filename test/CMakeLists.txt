#C++ 14 needed for Google Test Framework
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(THIS ${PROJECT_NAME}Tests)

#Fetches google-test framework from the github repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# Path of the sources of the cellox interpreter
set(SOURCEPATH ../src/)

# testfiles
set(TESTSOURCES
assignment_operator.cc
binary_operator.cc
class.cc
comments.cc
escape_sequences.cc
for_loops.cc
functions.cc
index_operator.cc
literal_expressions.cc
logical_operators.cc
native_functions.cc
super.cc
test_cellox.cc
test_cellox.hh
unary_operator.cc
while_loops.cc
)

# source files of the interpreter
set(CELLOXSOURCES
${SOURCEPATH}chunk.c
${SOURCEPATH}compiler.c
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_array.c
${SOURCEPATH}debug.c
${SOURCEPATH}init.c 
${SOURCEPATH}lexer.c
${SOURCEPATH}memory.c 
${SOURCEPATH}native_fun.c
${SOURCEPATH}object.c
${SOURCEPATH}string_utils.c
${SOURCEPATH}table.c
${SOURCEPATH}value.c  
${SOURCEPATH}virtual_machine.c
)

# headers of the interpreter
set(CELLOXHEADERS
${SOURCEPATH}chunk.h
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_array.h
${SOURCEPATH}debug.h
${SOURCEPATH}init.h
${SOURCEPATH}lexer.h
${SOURCEPATH}memory.h
${SOURCEPATH}native_fun.h
${SOURCEPATH}object.h
${SOURCEPATH}string_utils.h
${SOURCEPATH}table.h
${SOURCEPATH}value.h
${SOURCEPATH}virtual_machine.h
)

add_compile_definitions(
# Flag that indicates that test are executed
CELLOX_TESTS_RUNNING
# Directory of the cellox integration tests
TEST_PROGRAM_BASE_PATH="${PROJECT_SOURCE_DIR}/test/"
)

# Precompiles common.h to speed up compilation of the target
if( MSVC )    
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${SOURCEPATH}common.h ${SOURCEPATH}common.c) # VisualStudio only accepts header files that also have a source file
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${SOURCEPATH}common.h)
endif()

# Builds cellox tests
add_executable(${THIS} ${TESTSOURCES} ${CELLOXSOURCES} ${CELLOXHEADERS})

# Includes SourcePath of the compiler for shorter includes
target_include_directories(${THIS} PUBLIC ${SOURCEPATH})

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#Fetches google test
FetchContent_MakeAvailable(googletest)

#Links googletest libary and cellox tests
target_link_libraries(${THIS} GTest::gtest_main)

#Includes google test framework
include(googletest)
# Automatic discovering of tests
gtest_discover_tests(${THIS})
