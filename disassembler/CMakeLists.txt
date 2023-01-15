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
"${SOURCEPATH}/chunk_file.c"
"${SOURCEPATH}/chunk_optimizer.c"
"${SOURCEPATH}/chunk.c"
"${SOURCEPATH}/common.c"
"${SOURCEPATH}/compiler.c"
"${SOURCEPATH}/dynamic_value_array.c"
"${SOURCEPATH}/debug.c"
"${SOURCEPATH}/garbage_collector.c"
"${SOURCEPATH}/hash_table.c"
"${SOURCEPATH}/lexer.c"
"${SOURCEPATH}/memory_mutator.c"
"${SOURCEPATH}/native_functions.c"
"${SOURCEPATH}/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/value.c"
"${SOURCEPATH}/virtual_machine.c"
)
set(DISASSEMBLER_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/chunk_file.h"
"${SOURCEPATH}/chunk_optimizer.h"
"${SOURCEPATH}/chunk.h"
"${SOURCEPATH}/compiler.h"
"${SOURCEPATH}/dynamic_value_array.h"
"${SOURCEPATH}/debug.h"
"${SOURCEPATH}/garbage_collector.h"
"${SOURCEPATH}/hash_table.h"
"${SOURCEPATH}/lexer.h"
"${SOURCEPATH}/memory_mutator.h"
"${SOURCEPATH}/native_functions.h"
"${SOURCEPATH}/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/value.h"
"${SOURCEPATH}/virtual_machine.h"
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