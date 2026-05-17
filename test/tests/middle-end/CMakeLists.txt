add_executable(test_optimization_passes test_optimization_passes.c)
target_include_directories(test_optimization_passes PRIVATE ${SOURCEPATH})
target_link_libraries(test_optimization_passes
    clx_middleend
    clx_bytecode
    clx_model
    clx_backend
)
add_test(NAME optimization_passes COMMAND $<TARGET_FILE:test_optimization_passes>)
set_tests_properties(optimization_passes PROPERTIES LABELS "unit")

add_executable(test_optimization_integration test_integration.c)
target_include_directories(test_optimization_integration PRIVATE ${SOURCEPATH})
target_link_libraries(test_optimization_integration
    clx_frontend
    clx_middleend
    clx_bytecode
    clx_model
    clx_backend
    clx_module_loading
)
add_test(NAME optimization_integration COMMAND $<TARGET_FILE:test_optimization_integration>)
set_tests_properties(optimization_integration PROPERTIES LABELS "integration")
