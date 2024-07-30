# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# Boosting us a bit

find_package(Boost ${Boost_MINIMUM_VERSION} MODULE REQUIRED)
set_package_properties(Boost PROPERTIES
    DESCRIPTION "Boost C++ Libraries"
    URL "https://www.boost.org"
    TYPE REQUIRED
)
include_directories (${Boost_INCLUDE_DIRS})

# =======================================================
# Starting the actual project definition

# Config file
set (KAMD_PLUGIN_VERSION 1)
set (KAMD_PLUGIN_DIR "kactivitymanagerd${KAMD_PLUGIN_VERSION}")
set (KAMD_FULL_PLUGIN_DIR "${KDE_INSTALL_FULL_PLUGINDIR}/${KAMD_PLUGIN_DIR}/")
configure_file (kactivities-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kactivities-features.h)

include_directories (
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   )

# D-Bus activation service file, and associated systemd service
ecm_generate_dbus_service_file(
   NAME org.kde.ActivityManager
   EXECUTABLE "${KDE_INSTALL_FULL_LIBEXECDIR}/kactivitymanagerd"
   SYSTEMD_SERVICE plasma-kactivitymanagerd.service
   DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)
ecm_install_configured_files(
   INPUT plasma-kactivitymanagerd.service.in
   DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)

add_library(debug_STATIC STATIC)
set_target_properties(debug_STATIC PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(debug_STATIC Qt::Core)
ecm_qt_declare_logging_category(debug_STATIC
   HEADER DebugActivities.h
   IDENTIFIER KAMD_LOG_ACTIVITIES
   CATEGORY_NAME org.kde.kactivities.activities
   DEFAULT_SEVERITY Warning
   DESCRIPTION "KActivities Activities" EXPORT KACTIVITYMANAGERD)
ecm_qt_declare_logging_category(debug_STATIC
   HEADER DebugResources.h
   IDENTIFIER KAMD_LOG_RESOURCES
   CATEGORY_NAME org.kde.kactivities.resources
   DEFAULT_SEVERITY Warning
   DESCRIPTION "KActivities Resources" EXPORT KACTIVITYMANAGERD)
ecm_qt_declare_logging_category(debug_STATIC
   HEADER DebugApplication.h
   IDENTIFIER KAMD_LOG_APPLICATION
   CATEGORY_NAME org.kde.kactivities.application
   DEFAULT_SEVERITY Warning
   DESCRIPTION "KActivities Application" EXPORT KACTIVITYMANAGERD)

add_subdirectory (service)
