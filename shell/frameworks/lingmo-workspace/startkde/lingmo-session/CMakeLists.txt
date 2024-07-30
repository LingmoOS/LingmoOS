add_subdirectory(lingmo-autostart-list)

set(lingmo_session_SRCS
    main.cpp
    autostart.cpp
    startup.cpp
    sessiontrack.cpp
    signalhandler.cpp
)

ecm_qt_declare_logging_category(lingmo_session_SRCS  HEADER debug.h IDENTIFIER LINGMO_SESSION CATEGORY_NAME org.kde.lingmo.session
    DESCRIPTION "lingmo session"
    EXPORT LINGMOWORKSPACE
)

qt_add_dbus_adaptor( lingmo_session_SRCS org.kde.Startup.xml startup.h Startup)

set(kcminit_adaptor ${lingmo-workspace_SOURCE_DIR}/startkde/kcminit/main.h)
set(kcminit_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCMinit.xml)
qt_generate_dbus_interface( ${kcminit_adaptor} ${kcminit_xml} )
qt_add_dbus_interface( lingmo_session_SRCS ${kcminit_xml} kcminit_interface )

qt_add_dbus_interface( lingmo_session_SRCS ../../ksmserver/org.kde.KSMServerInterface.xml ksmserver_interface )

add_executable(lingmo_session ${lingmo_session_SRCS})

target_include_directories(lingmo_session PRIVATE ${CMAKE_SOURCE_DIR}/startkde ${CMAKE_BINARY_DIR}/startkde)
target_link_libraries(lingmo_session
    startlingmo
    KF6::KIOCore
    LingmoAutostart
)

install(TARGETS lingmo_session ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

