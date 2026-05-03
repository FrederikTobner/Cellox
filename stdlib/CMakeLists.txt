set(CELLOX_STDLIB_FILES
    array.clx
    io.clx
    math.clx
    os.clx
    string.clx
)

# When installing, place the stdlib next to the binary so the runtime can find it.
install(FILES ${CELLOX_STDLIB_FILES} DESTINATION lib/cellox/stdlib)

# Expose the source directory as the stdlib search root for test builds so that
# no installation step is required to run the test suite.
set(CELLOX_STDLIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
