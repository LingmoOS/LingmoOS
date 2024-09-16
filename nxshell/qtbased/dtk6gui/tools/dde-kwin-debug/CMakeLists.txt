set(BIN_NAME dde-kwin-bug)
set(TARGET_NAME ${BIN_NAME}${DTK_VERSION_MAJOR})

pkg_check_modules(X11 IMPORTED_TARGET REQUIRED x11)

add_executable(${TARGET_NAME}
    main.cpp
)
target_link_libraries(${TARGET_NAME} PRIVATE
    PkgConfig::X11
    pthread
)
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BIN_NAME})

install(TARGETS ${TARGET_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
