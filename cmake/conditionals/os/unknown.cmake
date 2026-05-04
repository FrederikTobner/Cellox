set(CLX_OS_LAYER_DIR "")
add_compile_definitions(OS_UNKNOWN)

function(clx_os_check_core_headers)
    message(FATAL_ERROR "Unsupported OS for clx_os layer")
endfunction()

function(clx_os_check_benchmark_headers)
    message(FATAL_ERROR "Unsupported OS for benchmark headers")
endfunction()