add_definitions(-DTRANSLATION_DOMAIN=\"kcm_pulseaudio\")

ecm_add_qml_module(lingmo-volume-declarative URI org.kde.lingmo.private.volume)

set(CMAKE_CXX_VISIBILITY_PRESET default)

set(lingmo-volume_SRCS
    modulemanager.cpp
    volumemonitor.cpp
    debug.cpp
    canberracontext.cpp
    speakertest.cpp
    gsettingsitem.cpp
    soundthemeconfig.cpp
    volumefeedback.cpp
    audioicon.cpp
    preferreddevice.cpp
)

add_library(lingmo-volume
    ${lingmo-volume_SRCS}
)

set_target_properties(lingmo-volume PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR}
                      EXPORT_NAME LibLingmoVolume)

target_sources(lingmo-volume-declarative PRIVATE
    ${lingmo-volume_SRCS}
    qml/listitemmenu.cpp
    qml/plugin.cpp
    qml/microphoneindicator.cpp
    qml/globalservice.cpp
)

ecm_target_qml_sources(lingmo-volume-declarative VERSION 0.1 SOURCES
    qml/PulseObjectFilterModel.qml
)

set_property(SOURCE dbus/osdService.xml APPEND PROPERTY CLASSNAME OsdServiceInterface)
qt_add_dbus_interface(dbus_SRCS dbus/osdService.xml osdservice)

ecm_generate_export_header(lingmo-volume-declarative
    BASE_NAME PulseAudioQt
    GROUP_BASE_NAME KF
    VERSION ${PROJECT_VERSION}
)

target_sources(lingmo-volume-declarative PRIVATE ${dbus_SRCS})
target_sources(lingmo-volume PRIVATE ${dbus_SRCS})

kconfig_add_kcfg_files(lingmo-volume-declarative GENERATE_MOC globalconfig.kcfgc)
kconfig_add_kcfg_files(lingmo-volume GENERATE_MOC globalconfig.kcfgc)

target_link_libraries(lingmo-volume-declarative PRIVATE
    Qt::Core
    Qt::Gui
    Qt::DBus
    Qt::Quick
    Qt::Widgets
    KF6::GlobalAccel
    KF6::I18n
    KF6::StatusNotifierItem
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::PulseAudioQt
    Canberra::Canberra
    PkgConfig::LIBPULSE
    PkgConfig::LIBPULSE_MAINLOOP
    PkgConfig::GIO
)

target_link_libraries(lingmo-volume PRIVATE
    Qt::Core
    Qt::Gui
    Qt::DBus
    Qt::Quick
    Qt::Widgets
    KF6::GlobalAccel
    KF6::I18n
    KF6::StatusNotifierItem
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::PulseAudioQt
    Canberra::Canberra
    PkgConfig::LIBPULSE
    PkgConfig::LIBPULSE_MAINLOOP
    PkgConfig::GIO
)

ecm_finalize_qml_module(lingmo-volume-declarative)
install(TARGETS lingmo-volume ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

add_subdirectory(kcm)
add_subdirectory(kded)
