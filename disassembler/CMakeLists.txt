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
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/backend/garbage_collector.c"
"${SOURCEPATH}/backend/memory_mutator.c"
"${SOURCEPATH}/backend/native_functions.c"
"${SOURCEPATH}/backend/virtual_machine.c"
"${SOURCEPATH}/byte-code/chunk.c"
"${SOURCEPATH}/byte-code/chunk_disassembler.c"
"${SOURCEPATH}/byte-code/chunk_file.c"
"${SOURCEPATH}/frontend/compiler.c"
"${SOURCEPATH}/frontend/lexer.c"
"${SOURCEPATH}/language-models/object.c"
"${SOURCEPATH}/language-models/value.c"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.c"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.c"
"${SOURCEPATH}/middle-end/chunk_optimizer.c"
)

set(DISASSEMBLER_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/backend/garbage_collector.h"
"${SOURCEPATH}/backend/memory_mutator.h"
"${SOURCEPATH}/backend/native_functions.h"
"${SOURCEPATH}/backend/virtual_machine.h"
"${SOURCEPATH}/byte-code/chunk.h"
"${SOURCEPATH}/byte-code/chunk_disassembler.h"
"${SOURCEPATH}/byte-code/chunk_file.h"
"${SOURCEPATH}/frontend/compiler.h"
"${SOURCEPATH}/frontend/lexer.h"
"${SOURCEPATH}/language-models/object.h"
"${SOURCEPATH}/language-models/value.h"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.h"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.h"
"${SOURCEPATH}/middle-end/chunk_optimizer.h"
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