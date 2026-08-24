kcoreaddons_add_plugin(kcm_lingmodecoration SOURCES kcm_breezedecoration.cpp INSTALL_NAMESPACE "plasma/kcms/lingmo")
target_include_directories(kcm_lingmodecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_lingmodecoration lingmocommon5 lingmodecoration_STATIC KF5::I18n KF5::CoreAddons KF5::ConfigWidgets KF5::I18n
        KF5::IconThemes Qt::DBus Qt::Widgets)
kcmutils_generate_desktop_file(kcm_lingmodecoration)

#install bundled presets
install(DIRECTORY presets  DESTINATION  ${KDE_INSTALL_QTPLUGINDIR}/plasma/kcms/lingmo)
