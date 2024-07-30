include_directories (
    ${PowerDevil_SOURCE_DIR}/daemon
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_BINARY_DIR}/common
)

add_subdirectory(common)
add_subdirectory(profiles)
