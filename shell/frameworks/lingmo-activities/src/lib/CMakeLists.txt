# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# =======================================================
# Now that we finished with the boilerplate, start
# with the library definition

add_library(LingmoActivities)
add_library(Lingmo::Activities ALIAS LingmoActivities)

set_target_properties(LingmoActivities PROPERTIES
   VERSION     ${LINGMO_ACTIVITIES_VERSION}
   SOVERSION   ${LINGMO_ACTIVITIES_SOVERSION}
   EXPORT_NAME Activities
)

target_sources(LingmoActivities PRIVATE
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

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

   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/dbusfuture_p.cpp

   version.cpp
   version.h
)

set_source_files_properties (
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   PROPERTIES
   INCLUDE ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
   )

set(LingmoActivities_DBus_SRCS)
qt_add_dbus_interface(LingmoActivities_DBus_SRCS
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
)
qt_add_dbus_interface(LingmoActivities_DBus_SRCS
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
)
qt_add_dbus_interface(LingmoActivities_DBus_SRCS
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
)
qt_add_dbus_interface(LingmoActivities_DBus_SRCS
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resources_linking_interface
)
qt_add_dbus_interface(LingmoActivities_DBus_SRCS
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Application.xml
   application_interface
)
target_sources(LingmoActivities PRIVATE
   ${LingmoActivities_DBus_SRCS}
)

ecm_qt_declare_logging_category(LingmoActivities
    HEADER debug_p.h
    IDENTIFIER KAMD_CORELIB
    CATEGORY_NAME kde.lingmo.activities
    OLD_CATEGORY_NAMES org.kde.kactivities.lib.core
    DEFAULT_SEVERITY Warning
    DESCRIPTION "Lingmo Activities core lib"
    EXPORT LINGMO_ACTIVITIES
)

target_link_libraries(LingmoActivities
   PUBLIC
      Qt6::Core
   PRIVATE
      Qt6::DBus
)

set(LINGMO_ACTIVITIES_BUILD_INCLUDE_DIRS
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/
)

target_include_directories(LingmoActivities
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/LingmoActivities>"
   PRIVATE  ${LINGMO_ACTIVITIES_BUILD_INCLUDE_DIRS}
)

# install
ecm_generate_export_header (LingmoActivities
   BASE_NAME Lingmo_Activities
   VERSION ${PROJECT_VERSION}
   USE_VERSION_HEADER lingmo_activities_version.h
   DEPRECATED_BASE_VERSION 0
)

ecm_generate_headers (
   LingmoActivities_CamelCase_HEADERS
   HEADER_NAMES
   Consumer
   Controller
   Info
   ResourceInstance
   ActivitiesModel
   Version
   PREFIX LingmoActivities
   REQUIRED_HEADERS LingmoActivities_HEADERS
   )
install (
   FILES ${LingmoActivities_CamelCase_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/LingmoActivities/LingmoActivities
   COMPONENT Devel
   )

install (
   FILES ${LingmoActivities_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/lingmo_activities_export.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/LingmoActivities/lingmoactivities
   COMPONENT Devel
   )

install (
   TARGETS LingmoActivities
   EXPORT LingmoActivitiesLibraryTargets
   ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
   )

if(BUILD_QCH)
    ecm_add_qch(
        LingmoActivities_QCH
        NAME LingmoActivities
        BASE_NAME LingmoActivities
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${LingmoActivities_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${LINGMO_ACTIVITIES_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            LINGMO_ACTIVITIES_EXPORT
            LINGMO_ACTIVITIES_DEPRECATED
            LINGMO_ACTIVITIES_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()


if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME LingmoActivities
      LIB_NAME LingmoActivities
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/LingmoActivities
      DEPS Qt6Core
      DESCRIPTION "Lingmo Activities is a C++ library for using Lingmo's activities"
      INSTALL
    )
endif ()

