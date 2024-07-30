add_definitions(-DTRANSLATION_DOMAIN=\"powerdevil\" -DQT_USE_QSTRINGBUILDER)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(actions)

# target no.1 - powerdevil core library
set(powerdevilcore_SRCS
    powerdevil_debug.cpp
    powerdevilaction.cpp
    powerdevilcore.cpp
    powerdevilenums.cpp
    powerdevilmigrateconfig.cpp
    powerdevilpolicyagent.cpp
    powerdevilbrightnesslogic.cpp
    powerdevilscreenbrightnesslogic.cpp
    powerdevilkeyboardbrightnesslogic.cpp
    powerdevilpowermanagement.cpp
    powerdevilsettingsdefaults.cpp
    brightnessosdwidget.cpp
    kwinkscreenhelpereffect.cpp
    upowerdevice.cpp
    controllers/lidcontroller.cpp
    controllers/batterycontroller.cpp
    controllers/suspendcontroller.cpp
    controllers/keyboardbrightnesscontroller.cpp
    controllers/screenbrightnesscontroller.cpp
    controllers/displaybrightness.cpp
    controllers/backlightbrightness.cpp
    controllers/ddcutildetector.cpp
    controllers/ddcutildisplay.cpp
    controllers/udevqtclient.cpp
    controllers/udevqtdevice.cpp
    controllers/kwinbrightness.cpp
    controllers/externalbrightnesscontrol.cpp
)

add_library(powerdevil_log_static STATIC)
set_property(TARGET powerdevil_log_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(powerdevil_log_static Qt::Core)
ecm_qt_declare_logging_category(powerdevil_log_static
   HEADER powerdevil_debug.h
   IDENTIFIER POWERDEVIL
   CATEGORY_NAME org.kde.powerdevil
   DEFAULT_SEVERITY Warning
   DESCRIPTION "Powerdevil" EXPORT POWERDEVIL
)

kconfig_add_kcfg_files(powerdevilcore_SRCS
    ${CMAKE_SOURCE_DIR}/PowerDevilGlobalSettings.kcfgc
    ${CMAKE_SOURCE_DIR}/PowerDevilProfileSettings.kcfgc
    ${CMAKE_SOURCE_DIR}/PowerDevilActivitySettings.kcfgc
    GENERATE_MOC
)

# Action DBus Adaptors

set_source_files_properties(
        dbus/org.freedesktop.UPower.xml
        dbus/org.freedesktop.UPower.Device.xml
        PROPERTIES NO_NAMESPACE TRUE
)

qt_add_dbus_interface(powerdevilcore_SRCS dbus/org.freedesktop.ScreenSaver.xml screenlocker_interface)

qt_add_dbus_interface(powerdevilcore_SRCS dbus/org.freedesktop.UPower.xml upower_interface)

qt_add_dbus_interface(powerdevilcore_SRCS dbus/org.freedesktop.UPower.KbdBacklight.xml upower_kbdbacklight_interface)

add_library(powerdevilcore SHARED ${powerdevilcore_SRCS})
set_target_properties(powerdevilcore PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 2)

qt6_generate_wayland_protocol_client_sources(powerdevilcore
    FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-external-brightness-v1.xml
)

generate_export_header(powerdevilcore)

# not exported, so just make the deps public
target_link_libraries(powerdevilcore
    Plasma::Activities
    Qt::DBus
    PW::KWorkspace
    KF6::CoreAddons
    KF6::AuthCore
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::DBusAddons
    KF6::I18n
    KF6::Solid
    KF6::IdleTime
    KF6::GlobalAccel
    KF6::Screen
    KF6::Service
    KF6::Notifications
    KF6::XmlGui
    KF6::KirigamiPlatform
    XCB::DPMS
    powerdevil_log_static
    ${UDEV_LIBS}
    Qt::WaylandClient
)

if (XCB_FOUND) # kwin kscreen helper effect
    target_link_libraries(powerdevilcore XCB::XCB Qt::GuiPrivate)
endif ()

if(DDCUTIL_FOUND)
    target_link_libraries(powerdevilcore ${LIBDDCUTIL_LIBRARY})
endif()

# target no.2 - powerdevil kded module
set(powerdevil_SRCS
    powerdevil_debug.cpp
    powerdevilapp.cpp
    powerdevilfdoconnector.cpp
)

# DBus Adaptors
qt_add_dbus_adaptor(powerdevil_SRCS dbus/org.kde.Solid.PowerManagement.xml powerdevilcore.h PowerDevil::Core)
qt_add_dbus_adaptor(powerdevil_SRCS dbus/org.kde.Solid.PowerManagement.PolicyAgent.xml
                                         powerdevilpolicyagent.h PowerDevil::PolicyAgent
                                         powermanagementpolicyagentadaptor PowerManagementPolicyAgentAdaptor)

qt_add_dbus_adaptor(powerdevil_SRCS dbus/org.freedesktop.PowerManagement.xml powerdevilfdoconnector.h PowerDevil::FdoConnector powermanagementfdoadaptor PowerManagementFdoAdaptor)
qt_add_dbus_adaptor(powerdevil_SRCS dbus/org.freedesktop.PowerManagement.Inhibit.xml powerdevilfdoconnector.h PowerDevil::FdoConnector powermanagementinhibitadaptor PowerManagementInhibitAdaptor)

# KAuth charge threshold helper
add_executable(chargethresholdhelper powerdevil_debug.cpp)
if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    target_sources(chargethresholdhelper PRIVATE chargethresholdhelper_openbsd.cpp)
else()
    target_sources(chargethresholdhelper PRIVATE chargethresholdhelper_linux.cpp)
endif()
target_link_libraries(chargethresholdhelper Qt::Core KF6::AuthCore)
install(TARGETS chargethresholdhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(chargethresholdhelper org.kde.powerdevil.chargethresholdhelper root)
kauth_install_actions(org.kde.powerdevil.chargethresholdhelper chargethreshold_helper_actions.actions)

## discrete gpu helper executable
add_executable(discretegpuhelper discretegpuhelper.cpp ${CMAKE_BINARY_DIR}/daemon/powerdevil_debug.cpp)
target_link_libraries(discretegpuhelper Qt::Core KF6::AuthCore)
install(TARGETS discretegpuhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(discretegpuhelper org.kde.powerdevil.discretegpuhelper root)
kauth_install_actions(org.kde.powerdevil.discretegpuhelper discretegpu_helper_actions.actions)


## backlight helper executable
set(backlighthelpers_SRCS ${CMAKE_BINARY_DIR}/daemon/powerdevil_debug.cpp)
if (CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    list(APPEND backlighthelpers_SRCS controllers/backlighthelper_freebsd.cpp)
else()
    list(APPEND backlighthelpers_SRCS controllers/backlighthelper_linux.cpp)
endif()

add_executable(backlighthelper ${backlighthelpers_SRCS})
target_link_libraries(backlighthelper Qt::Core KF6::AuthCore KF6::I18n)
install(TARGETS backlighthelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(backlighthelper org.kde.powerdevil.backlighthelper root)
kauth_install_actions(org.kde.powerdevil.backlighthelper controllers/backlight_helper_actions.actions)


add_executable(powerdevil ${powerdevil_SRCS})
set_target_properties(powerdevil PROPERTIES OUTPUT_NAME org_kde_powerdevil)
configure_file(powerdevil.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/powerdevil.desktop)

target_link_libraries(powerdevil
    KF6::Crash
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::DBusAddons
    PW::KWorkspace
    powerdevilcore
)
if (XCB_FOUND)
    target_link_libraries(powerdevil Qt::GuiPrivate)
endif ()

target_link_libraries(powerdevil ${UDEV_LIBS})

install(TARGETS powerdevil DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
if (HAVE_LIBCAP)
    install(
    CODE "execute_process(
            COMMAND
                ${SETCAP_EXECUTABLE}
                CAP_WAKE_ALARM=+ep
                \$ENV{DESTDIR}${KDE_INSTALL_FULL_LIBEXECDIR}/org_kde_powerdevil)"
    )
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/powerdevil.desktop
        DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
install(TARGETS powerdevilcore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

set(HAVE_XCB ${XCB_FOUND})
configure_file(config-powerdevil.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powerdevil.h )

ecm_install_configured_files(INPUT plasma-powerdevil.service.in @ONLY DESTINATION
        ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
