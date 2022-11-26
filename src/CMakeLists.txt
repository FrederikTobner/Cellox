cmake_minimum_required(VERSION 3.16)
# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

add_executable(${PROJECT_NAME} ${CELLOXSOURCES} ${CELLOXHEADERS} main.c)

# Precompiles common.h to speed up compilation of the target
if(MSVC)
    # VisualStudio only accepts header files that also have a source file    
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h common.c) 
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h)
endif()

# Includes Libmath under unix-like systems
if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()

if(NAN_BOXING_ACTIVATED)
add_compile_definitions(
NAN_BOXING
)
endif()

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)

# Sets properties for the package created using cpack
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Interpreter")
set(CPACK_BUILD_SOURCE_DIRS "${PROJECT_BINARY_DIR}/src")

include(CPack)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)