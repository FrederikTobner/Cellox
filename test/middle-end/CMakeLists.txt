add_executable(test_optimization_passes test_optimization_passes.c)
target_include_directories(test_optimization_passes PRIVATE ${SOURCEPATH})
target_link_libraries(test_optimization_passes
    clx_middleend
    clx_bytecode
    clx_model
    clx_runtime
)
add_test(NAME optimization_passes COMMAND test_optimization_passes)

add_executable(test_optimization_integration test_integration.c)
target_include_directories(test_optimization_integration PRIVATE ${SOURCEPATH})
target_link_libraries(test_optimization_integration
    clx_frontend
    clx_middleend
    clx_bytecode
    clx_model
    clx_runtime
    clx_module_loading
)
add_test(NAME optimization_integration COMMAND test_optimization_integration)
