set(LANGUAGE_DISASSEMBLER ${PROJECT_NAME}Disassembler)

set(DISASSEMBLER_HEADER_FILES
"disassembler.c"
)

set(DISASSEMBLER_SOURCE_FILES
"main.c"
"disassembler.h"
)

# dependencies from the interpreter needed to build the disassembler tool
set(DISASSEMBLER_DEPENDENCIES_SOURCE_FILES
"${SOURCEPATH}/backend/chunk_file.c"
"${SOURCEPATH}/middleend/chunk_optimizer.c"
"${SOURCEPATH}/frontend/chunk.c"
"${SOURCEPATH}/common.c"
"${SOURCEPATH}/frontend/compiler.c"
"${SOURCEPATH}/backend/dynamic_value_array.c"
"${SOURCEPATH}/backend/debug.c"
"${SOURCEPATH}/backend/garbage_collector.c"
"${SOURCEPATH}/backend/hash_table.c"
"${SOURCEPATH}/frontend/lexer.c"
"${SOURCEPATH}/backend/memory_mutator.c"
"${SOURCEPATH}/backend/native_functions.c"
"${SOURCEPATH}/backend/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/backend/value.c"
"${SOURCEPATH}/backend/virtual_machine.c"
)
set(DISASSEMBLER_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/backend/chunk_file.h"
"${SOURCEPATH}/middleend/chunk_optimizer.h"
"${SOURCEPATH}/frontend/chunk.h"
"${SOURCEPATH}/frontend/compiler.h"
"${SOURCEPATH}/backend/dynamic_value_array.h"
"${SOURCEPATH}/backend/debug.h"
"${SOURCEPATH}/backend/garbage_collector.h"
"${SOURCEPATH}/backend/hash_table.h"
"${SOURCEPATH}/frontend/lexer.h"
"${SOURCEPATH}/backend/memory_mutator.h"
"${SOURCEPATH}/backend/native_functions.h"
"${SOURCEPATH}/backend/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/backend/value.h"
"${SOURCEPATH}/backend/virtual_machine.h"
)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_compile_definitions(BUILD_DEBUG)
endif()

add_executable(${LANGUAGE_DISASSEMBLER} 
${DISASSEMBLER_DEPENDENCIES_SOURCE_FILES} ${DISASSEMBLER_DEPENDENCIES_HEADER_FILES} 
${DISASSEMBLER_SOURCE_FILES} ${DISASSEMBLER_HEADER_FILES})

if(UNIX)
    target_link_libraries(${LANGUAGE_DISASSEMBLER} m)
endif()

target_include_directories(${LANGUAGE_DISASSEMBLER} PUBLIC ${PROJECT_BINARY_DIR}/src)