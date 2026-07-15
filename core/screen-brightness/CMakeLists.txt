project(lingmo-screen-brightness)
set(PROJECT_NAME lingmo-screen-brightness)

set(SRCS
    main.cpp
    brightnesshelper.h
    brightnesshelper.cpp
)

add_executable(${PROJECT_NAME}
    ${SRCS}
)

target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

configure_file(
    com.lingmo.brightness.pkexec.policy.in
    com.lingmo.brightness.pkexec.policy
    @ONLY
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/com.lingmo.brightness.pkexec.policy DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions/)
