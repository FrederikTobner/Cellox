set(LANGUAGE_DISASSEMBLER ${PROJECT_NAME}Disassembler)

cellox_add_compiler_compile_definitions()
cellox_add_os_compile_definitions()

file(GLOB_RECURSE DISASSEMBLER_HEADER_FILES *.h)
file(GLOB_RECURSE DISASSEMBLER_SOURCE_FILES *.c)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_compile_definitions(BUILD_TYPE_DEBUG)
endif()

cellox_add_executable(${LANGUAGE_DISASSEMBLER}
    SOURCES
        ${DISASSEMBLER_SOURCE_FILES}
    DEPENDENCIES
        cellox_all
)