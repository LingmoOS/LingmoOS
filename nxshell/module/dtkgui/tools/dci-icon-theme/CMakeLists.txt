set(BIN_NAME dci-icon-theme)
set(TARGET_NAME ${BIN_NAME}${DTK_VERSION_MAJOR})

add_executable(${TARGET_NAME}
    main.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    ${LIB_NAME}
)
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BIN_NAME})

install(TARGETS ${TARGET_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
