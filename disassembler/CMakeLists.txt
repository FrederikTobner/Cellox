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
"${SOURCEPATH}/byte-code/chunk_file.c"
"${SOURCEPATH}/middle-end/chunk_optimizer.c"
"${SOURCEPATH}/byte-code/chunk.c"
"${SOURCEPATH}/common.c"
"${SOURCEPATH}/frontend/compiler.c"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.c"
"${SOURCEPATH}/byte-code/chunk_disassembler.c"
"${SOURCEPATH}/backend/garbage_collector.c"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.c"
"${SOURCEPATH}/frontend/lexer.c"
"${SOURCEPATH}/backend/memory_mutator.c"
"${SOURCEPATH}/backend/native_functions.c"
"${SOURCEPATH}/language-models/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/language-models/value.c"
"${SOURCEPATH}/backend/virtual_machine.c"
)
set(DISASSEMBLER_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/byte-code/chunk_file.h"
"${SOURCEPATH}/middle-end/chunk_optimizer.h"
"${SOURCEPATH}/byte-code/chunk.h"
"${SOURCEPATH}/frontend/compiler.h"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.h"
"${SOURCEPATH}/byte-code/chunk_disassembler.h"
"${SOURCEPATH}/backend/garbage_collector.h"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.h"
"${SOURCEPATH}/frontend/lexer.h"
"${SOURCEPATH}/backend/memory_mutator.h"
"${SOURCEPATH}/backend/native_functions.h"
"${SOURCEPATH}/language-models/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/language-models/value.h"
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