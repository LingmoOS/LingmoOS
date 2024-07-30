set(lingmo_shutdown_SRCS
    main.cpp
    shutdown.cpp
)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ecm_qt_declare_logging_category(lingmo_shutdown_SRCS  HEADER debug.h IDENTIFIER LINGMO_SESSION CATEGORY_NAME org.kde.lingmo.shutdown
    DESCRIPTION "lingmo shutdown"
    EXPORT LINGMOWORKSPACE
)

qt_add_dbus_adaptor(lingmo_shutdown_SRCS org.kde.Shutdown.xml shutdown.h Shutdown)
qt_add_dbus_interface(lingmo_shutdown_SRCS org.kde.Shutdown.xml shutdown_interface)
qt_add_dbus_interface( lingmo_shutdown_SRCS ../../ksmserver/org.kde.KSMServerInterface.xml ksmserver_interface )
qt_add_dbus_interface( lingmo_shutdown_SRCS ../../ksmserver/org.kde.KWin.Session.xml kwin_interface )

add_executable(lingmo-shutdown ${lingmo_shutdown_SRCS})

target_link_libraries(lingmo-shutdown
    Qt::Core
    Qt::DBus
    KF6::ConfigCore
    PW::KWorkspace
)

kdbusaddons_generate_dbus_service_file(lingmo-shutdown org.kde.Shutdown ${KDE_INSTALL_FULL_BINDIR})
install(TARGETS lingmo-shutdown ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

