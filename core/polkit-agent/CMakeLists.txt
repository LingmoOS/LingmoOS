find_package(PkgConfig)
pkg_check_modules(POLKIT_AGENT REQUIRED polkit-agent-1)
message(STATUS "polkit agent: ${POLKIT_AGENT_INCLUDE_DIRS} ${POLKIT_AGENT_LIBRARIES}")

find_package(PolkitQt6-1 REQUIRED)

include_directories(
    ${POLKIT_AGENT_INCLUDE_DIRS}
)

add_executable(lingmo-polkit-agent
    main.cpp
    polkitagentlistener.cpp
    dialog.cpp
    qml.qrc
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

target_link_libraries(lingmo-polkit-agent
        Qt6::Core
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2

        # LingmoUI

        PolkitQt6-1::Core
        PolkitQt6-1::Agent
        ${POLKIT_AGENT_LDFLAGS}
)

add_custom_target(polkit-agent-translations DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})
add_dependencies(lingmo-polkit-agent polkit-agent-translations)

install(TARGETS lingmo-polkit-agent
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Runtime
)

install(FILES
lingmo-polkit-agent.desktop
    DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/autostart"
    COMPONENT Runtime
)

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/lingmo-polkit-agent/translations)
