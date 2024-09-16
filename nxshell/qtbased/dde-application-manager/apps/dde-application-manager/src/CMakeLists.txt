set(BIN_NAME dde-application-manager)

add_executable(${BIN_NAME} main.cpp utils.cpp)

target_link_libraries(${BIN_NAME} PRIVATE
    dde_am_static
)

target_include_directories(${BIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
