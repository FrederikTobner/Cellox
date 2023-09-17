set(LANGUAGE_DISASSEMBLER ${PROJECT_NAME}Disassembler)

cellox_add_compiler_compile_definitions()
cellox_add_os_compile_definitions()

file(GLOB_RECURSE DISASSEMBLER_HEADER_FILES *.h)
file(GLOB_RECURSE DISASSEMBLER_SOURCE_FILES *.c)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_compile_definitions(BUILD_TYPE_DEBUG)
endif()

add_executable(${LANGUAGE_DISASSEMBLER} 
${CELLOX_SOURCE_FILES} ${CELLOX_HEADER_FILES} 
${DISASSEMBLER_SOURCE_FILES} ${DISASSEMBLER_HEADER_FILES})

if(UNIX)
    target_link_libraries(${LANGUAGE_DISASSEMBLER} m)
endif()

target_include_directories(${LANGUAGE_DISASSEMBLER} PUBLIC ${PROJECT_BINARY_DIR}/src)