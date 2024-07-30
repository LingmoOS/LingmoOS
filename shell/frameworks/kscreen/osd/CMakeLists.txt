add_executable(kscreen_osd_service main.cpp osdmanager.cpp osd.cpp ../common/osdaction.cpp qml.qrc)

qt_add_dbus_adaptor(DBUS_SRC org.kde.kscreen.osdService.xml osdmanager.h KScreen::OsdManager)
target_sources(kscreen_osd_service PRIVATE ${DBUS_SRC})

target_link_libraries(kscreen_osd_service PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::I18n
    KF6::WindowSystem
    KF6::Screen
    LayerShellQt::Interface
)

install(TARGETS kscreen_osd_service DESTINATION ${KDE_INSTALL_LIBEXECDIR})

ecm_generate_dbus_service_file(
    NAME org.kde.kscreen.osdService
    EXECUTABLE ${KDE_INSTALL_FULL_LIBEXECDIR}/kscreen_osd_service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
    SYSTEMD_SERVICE plasma-kscreen-osd.service
)

ecm_install_configured_files(INPUT plasma-kscreen-osd.service @ONLY
    DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)

install(FILES org.kde.kscreen.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
