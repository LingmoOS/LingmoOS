# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# =======================================================
# Now that we finished with the boilerplate, start
# with the library definition

add_library(PlasmaActivities)
add_library(Plasma::Activities ALIAS PlasmaActivities)

set_target_properties(PlasmaActivities PROPERTIES
   VERSION     ${PLASMA_ACTIVITIES_VERSION}
   SOVERSION   ${PLASMA_ACTIVITIES_SOVERSION}
   EXPORT_NAME Activities
)

target_sources(PlasmaActivities PRIVATE
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   consumer.cpp
   consumer.h
   controller.cpp
   controller.h
   info.cpp
   info.h
   resourceinstance.cpp
   resourceinstance.h
   activitiesmodel.cpp
   activitiesmodel.h

   mainthreadexecutor_p.cpp
   mainthreadexecutor_p.h
   manager_p.cpp
   manager_p.h
   activitiescache_p.cpp
   activitiescache_p.h

   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/dbusfuture_p.cpp

   version.cpp
   version.h
)

set_source_files_properties (
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   PROPERTIES
   INCLUDE ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
   )

set(PlasmaActivities_DBus_SRCS)
qt_add_dbus_interface(PlasmaActivities_DBus_SRCS
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
)
qt_add_dbus_interface(PlasmaActivities_DBus_SRCS
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
)
qt_add_dbus_interface(PlasmaActivities_DBus_SRCS
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
)
qt_add_dbus_interface(PlasmaActivities_DBus_SRCS
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resources_linking_interface
)
qt_add_dbus_interface(PlasmaActivities_DBus_SRCS
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Application.xml
   application_interface
)
target_sources(PlasmaActivities PRIVATE
   ${PlasmaActivities_DBus_SRCS}
)

ecm_qt_declare_logging_category(PlasmaActivities
    HEADER debug_p.h
    IDENTIFIER KAMD_CORELIB
    CATEGORY_NAME kde.plasma.activities
    OLD_CATEGORY_NAMES org.kde.kactivities.lib.core
    DEFAULT_SEVERITY Warning
    DESCRIPTION "Plasma Activities core lib"
    EXPORT PLASMA_ACTIVITIES
)

target_link_libraries(PlasmaActivities
   PUBLIC
      Qt6::Core
   PRIVATE
      Qt6::DBus
)

set(PLASMA_ACTIVITIES_BUILD_INCLUDE_DIRS
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/
)

target_include_directories(PlasmaActivities
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/PlasmaActivities>"
   PRIVATE  ${PLASMA_ACTIVITIES_BUILD_INCLUDE_DIRS}
)

# install
ecm_generate_export_header (PlasmaActivities
   BASE_NAME Plasma_Activities
   VERSION ${PROJECT_VERSION}
   USE_VERSION_HEADER plasma_activities_version.h
   DEPRECATED_BASE_VERSION 0
)

ecm_generate_headers (
   PlasmaActivities_CamelCase_HEADERS
   HEADER_NAMES
   Consumer
   Controller
   Info
   ResourceInstance
   ActivitiesModel
   Version
   PREFIX PlasmaActivities
   REQUIRED_HEADERS PlasmaActivities_HEADERS
   )
install (
   FILES ${PlasmaActivities_CamelCase_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivities/PlasmaActivities
   COMPONENT Devel
   )

install (
   FILES ${PlasmaActivities_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/plasma_activities_export.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivities/plasmaactivities
   COMPONENT Devel
   )

install (
   TARGETS PlasmaActivities
   EXPORT PlasmaActivitiesLibraryTargets
   ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
   )

if(BUILD_QCH)
    ecm_add_qch(
        PlasmaActivities_QCH
        NAME PlasmaActivities
        BASE_NAME PlasmaActivities
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${PlasmaActivities_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${PLASMA_ACTIVITIES_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            PLASMA_ACTIVITIES_EXPORT
            PLASMA_ACTIVITIES_DEPRECATED
            PLASMA_ACTIVITIES_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()


if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME PlasmaActivities
      LIB_NAME PlasmaActivities
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivities
      DEPS Qt6Core
      DESCRIPTION "Plasma Activities is a C++ library for using Plasma's activities"
      INSTALL
    )
endif ()

