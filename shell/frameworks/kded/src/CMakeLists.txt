add_executable(kded6)

target_sources(kded6 PRIVATE
    kded.cpp
    kdedadaptor.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded6.xml # just so that it gets generated
)

ecm_qt_declare_logging_category(
    kded6
    HEADER kded_debug.h
    IDENTIFIER KDED
    CATEGORY_NAME kf.kded
    OLD_CATEGORY_NAMES kf5.kded
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KDED"
    EXPORT KDED
)

ecm_mark_nongui_executable(kded6)

target_link_libraries(kded6
    Qt6::Widgets # QApplication
    KF6::Service # Needed for ksycoca.h
    KF6::CoreAddons # Needed for KDirWatch
    KF6::DBusAddons # Needed for kdedmodule.h
    KF6::Crash # Sets it as autostart
    KF6::ConfigCore
)

target_compile_definitions(kded6 PRIVATE
    KCONF_UPDATE_EXE="$<TARGET_FILE:KF6::kconf_update>"
)

install(TARGETS kded6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS} )

configure_file(plasma-kded6.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/plasma-kded6.service @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-kded6.service
        DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})

qt_generate_dbus_interface( kdedadaptor.h org.kde.kded6.xml )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded6.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})

kdbusaddons_generate_dbus_service_file(kded6 org.kde.kded6 ${KDE_INSTALL_FULL_BINDIR})

ecm_qt_install_logging_categories(
    EXPORT KDED
    FILE kded.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
