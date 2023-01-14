set(LANGUAGE_DISASSEMBLER ${PROJECT_NAME}Disassembler)

set(DISASSEMBLER_HEADER_FILES
"disassembler.c"
)

set(DISASSEMBLER_SOURCE_FILES
"main.c"
"disassembler.h"
)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_compile_definitions(BUILD_DEBUG)
endif()

add_executable(${LANGUAGE_DISASSEMBLER} ${CELLOX_SOURCE_FILES} ${CELLOX_HEADER_FILES} ${DISASSEMBLER_SOURCE_FILES} ${DISASSEMBLER_HEADER_FILES})

if(UNIX)
    target_link_libraries(${LANGUAGE_DISASSEMBLER} m)
endif()

target_include_directories(${LANGUAGE_DISASSEMBLER} PUBLIC ${PROJECT_BINARY_DIR}/src)