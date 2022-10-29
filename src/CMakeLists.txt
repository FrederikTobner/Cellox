# deletes build folder to ensure there are no makefiles that make the build fail (multiple target patterns)
if( MSVC )
execute_process(COMMAND ${CMAKE_COMMAND} "-E" "remove_directory" ${PROJECT_BINARY_DIR}/src)
endif()

# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

set(HEADERS
    chunk.h
    compiler.h
    dynamic_array.h
    debug.h
    init.h
    lexer.h
    memory.h
    native_functions.h
    object.h
    string_utils.h
    table.h
    value.h
    virtual_machine.h
)

set(SOURCES
    chunk.c
    compiler.c
    compiler.h
    dynamic_array.c
    debug.c
    init.c 
    lexer.c
    main.c
    memory.c 
    native_functions.c
    object.c 
    string_utils.c
    table.c
    value.c  
    virtual_machine.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Precompiles common.h to speed up compilation of the target
if( MSVC )    
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h common.c) # VisualStudio only accepts header files that also have a source file
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h)
endif()

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)
