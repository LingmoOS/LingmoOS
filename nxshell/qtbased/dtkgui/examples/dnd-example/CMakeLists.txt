find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(BIN1 dnd-test-client)
set(BIN2 dnd-test-server)
set(TARGET1 ${BIN1}${DTK_VERSION_MAJOR})
set(TARGET2 ${BIN2}${DTK_VERSION_MAJOR})

add_executable(${TARGET1}
    dnd-test-client.cpp
)

target_link_libraries(${TARGET1} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${LIB_NAME}
)

add_executable(${TARGET2}
    dnd-test-server.cpp
)

target_link_libraries(${TARGET2} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${LIB_NAME}
)

set_target_properties(${TARGET1} PROPERTIES OUTPUT_NAME ${BIN1})
set_target_properties(${TARGET2} PROPERTIES OUTPUT_NAME ${BIN2})

install(TARGETS ${TARGET1} DESTINATION "${TOOL_INSTALL_DIR}")
install(TARGETS ${TARGET2} DESTINATION "${TOOL_INSTALL_DIR}")
