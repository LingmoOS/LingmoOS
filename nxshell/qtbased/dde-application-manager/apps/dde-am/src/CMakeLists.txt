set(BIN_NAME dde-am)

add_executable(${BIN_NAME} main.cpp
    launcher.h launcher.cpp
)

target_link_libraries(${BIN_NAME} PRIVATE
    dde_am_static
    Dtk6::Core
)

target_include_directories(${BIN_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
