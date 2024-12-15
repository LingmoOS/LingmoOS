set(BIN_NAME filewatcher${DTK_VERSION_MAJOR})

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${BIN_NAME}
    main.cpp
)

target_link_libraries(
    ${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${LIB_NAME}
)

target_include_directories(${BIN_NAME} PUBLIC
    ../../include/filesystem/
    ../../include/
)
