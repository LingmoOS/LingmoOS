set(CHAMELEON_PATH "${PROJECT_BINARY_DIR}/plugins")
add_definitions(
    -DCHAMELEON_PATH="${CHAMELEON_PATH}"
    -DQT_DEPRECATED_WARNINGS
)
add_subdirectory(exhibition)
add_subdirectory(qml-inspect)
