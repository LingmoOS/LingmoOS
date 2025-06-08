add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kscreen\")

kcoreaddons_add_plugin(kcm_kscreen INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_sources(kcm_kscreen PRIVATE
    config_handler.cpp
    kcm.cpp
    output_model.cpp
    ${CMAKE_SOURCE_DIR}/common/utils.cpp
    ${CMAKE_SOURCE_DIR}/common/control.cpp
    ${CMAKE_SOURCE_DIR}/common/globals.cpp
    ${CMAKE_SOURCE_DIR}/common/orientation_sensor.cpp
)

ecm_qt_declare_logging_category(kcm_kscreen
    HEADER
        kcm_screen_debug.h
    IDENTIFIER
        KSCREEN_KCM
    CATEGORY_NAME
        kscreen.kcm
     DESCRIPTION "kscreen kcm (kscreen)" EXPORT KSCREEN
)

kconfig_add_kcfg_files(kcm_kscreen GENERATE_MOC globalscalesettings.kcfgc)

kcmutils_generate_desktop_file(kcm_kscreen)

target_link_libraries(kcm_kscreen
    Qt::DBus
    Qt::Sensors
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::Declarative
    KF5::I18n
    KF5::PlasmaQuick
    KF5::QuickAddons
    KF5::Screen
)

kpackage_install_package(package kcm_kscreen kcms)
