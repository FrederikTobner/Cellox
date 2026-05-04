add_compile_definitions(
    $<$<CONFIG:Debug>:BUILD_TYPE_DEBUG>
    $<$<CONFIG:Debug>:BUILD_DEBUG>
)

if(CLX_DEBUG_PRINT_BYTECODE)
    add_compile_definitions($<$<CONFIG:Debug>:DEBUG_PRINT_CODE>)
endif()

if(CLX_DEBUG_TRACE_EXECUTION)
    add_compile_definitions($<$<CONFIG:Debug>:DEBUG_TRACE_EXECUTION>)
endif()

if(CLX_DEBUG_STRESS_GARBAGE_COLLECTOR)
    add_compile_definitions($<$<CONFIG:Debug>:DEBUG_STRESS_GC>)
endif()

if(CLX_DEBUG_LOG_GARBAGE_COLLECTION OR CLX_DEBUG_LOG_GARBAGE_COLLECTOIN)
    add_compile_definitions($<$<CONFIG:Debug>:DEBUG_LOG_GC>)
endif()