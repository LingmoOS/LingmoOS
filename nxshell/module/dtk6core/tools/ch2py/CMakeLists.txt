set(TARGET_NAME ch2py)
set(BIN_NAME ${TARGET_NAME}${DTK_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(CMAKE_AUTORCC ON)
add_executable(${BIN_NAME}
    ${PROJECT_SOURCE_DIR}/src/util/dpinyin.cpp
    ${PROJECT_SOURCE_DIR}/src/util/util.qrc
    main.cpp
)
target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
target_include_directories(${BIN_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include/util
    ${PROJECT_SOURCE_DIR}/include/global
)
set_target_properties(${BIN_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
