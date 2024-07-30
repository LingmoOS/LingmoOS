# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (ActivityManager)

# General
find_package (ECM 0.0.8 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets)

find_package (KF6Config ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6CoreAddons ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6I18n ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6WindowSystem ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6GlobalAccel ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6XmlGui ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6KIO ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6Crash ${KF6_MIN_VERSION} CONFIG REQUIRED)


# Standard stuff

add_library(kactivitymanagerd_plugin SHARED Plugin.cpp Module.cpp Event.cpp)
generate_export_header(kactivitymanagerd_plugin)
target_link_libraries(kactivitymanagerd_plugin
    PUBLIC Qt::Core Qt::DBus KF6::CoreAddons KF6::ConfigCore
    PRIVATE debug_STATIC
)

add_subdirectory (plugins)

set (kactivitymanager_SRCS
   Application.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   Activities.cpp
   Resources.cpp
   Features.cpp
   Config.cpp

   ksmserver/KSMServer.cpp
   )

qt_add_dbus_adaptor (
   kactivitymanager_SRCS
   ../common/dbus/org.kde.ActivityManager.Activities.xml
   Activities.h Activities
   )

qt_add_dbus_adaptor (
   kactivitymanager_SRCS
   ../common/dbus/org.kde.ActivityManager.Resources.xml
   Resources.h Resources
   )

qt_add_dbus_adaptor (
   kactivitymanager_SRCS
   ../common/dbus/org.kde.ActivityManager.Features.xml
   Features.h Features
   )

add_executable (kactivitymanagerd ${kactivitymanager_SRCS})

target_link_libraries (kactivitymanagerd
   Qt::Core
   Qt::DBus
   Qt::Gui
   Qt::Widgets
   KF6::DBusAddons
   KF6::CoreAddons
   KF6::ConfigCore
   KF6::I18n
   KF6::WindowSystem
   KF6::Crash
   kactivitymanagerd_plugin
   debug_STATIC
   )

########### install application ###############

install (TARGETS
    kactivitymanagerd
    DESTINATION ${KDE_INSTALL_LIBEXECDIR}
   )

install (TARGETS
   kactivitymanagerd_plugin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
   )
