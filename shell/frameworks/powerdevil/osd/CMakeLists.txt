add_executable(power_profile_osd_service main.cpp osdmanager.cpp osdaction.cpp osd.cpp qml.qrc)
add_definitions(-DTRANSLATION_DOMAIN=\"powerdevil_osd\")

qt_add_dbus_adaptor(DBUS_SRC org.kde.powerdevil.powerProfileOsdService.xml osdmanager.h PowerDevil::OsdManager)
target_sources(power_profile_osd_service PRIVATE ${DBUS_SRC})

target_link_libraries(power_profile_osd_service PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::I18n
    KF6::WindowSystem
    KF6::Screen
    KF6::Crash
    LayerShellQt::Interface
)

install(TARGETS power_profile_osd_service DESTINATION ${KDE_INSTALL_LIBEXECDIR})

ecm_generate_dbus_service_file(
    NAME org.kde.powerdevil.powerProfileOsdService
    EXECUTABLE ${KDE_INSTALL_FULL_LIBEXECDIR}/power_profile_osd_service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
    SYSTEMD_SERVICE plasma-powerprofile-osd.service
)

ecm_install_configured_files(INPUT plasma-powerprofile-osd.service @ONLY
    DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)
