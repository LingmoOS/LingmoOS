# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

add_library(PlasmaActivitiesStats)
add_library(Plasma::ActivitiesStats ALIAS PlasmaActivitiesStats)

set_target_properties(PlasmaActivitiesStats PROPERTIES
   VERSION     ${PLASMAACTIVITIESSTATS_VERSION}
   SOVERSION   ${PLASMAACTIVITIESSTATS_SOVERSION}
   EXPORT_NAME ActivitiesStats
)

target_sources(PlasmaActivitiesStats PRIVATE
   query.cpp
   terms.cpp
   resultset.cpp
   resultwatcher.cpp
   resultmodel.cpp
   activitiessync_p.cpp
   cleaning.cpp

   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/common/database/Database.cpp
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp
)

ecm_qt_declare_logging_category(PlasmaActivitiesStats
    HEADER plasma-activities-stats-logsettings.h
    IDENTIFIER PLASMA_ACTIVITIES_STATS_LOG
    CATEGORY_NAME kde.plasma.activitiesstats
    OLD_CATEGORY_NAMES kf5.kactivity.stat kf.activitiesstats
    DESCRIPTION "Plasma Activities Stats"
    EXPORT PLASMA_ACTIVITIES_STATS
)

set(PlasmaActivitiesStats_DBus_SRCS)
qt_add_dbus_interface(PlasmaActivitiesStats_DBus_SRCS
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesScoring.xml
   resourcesscoring_interface
)
qt_add_dbus_interface(PlasmaActivitiesStats_DBus_SRCS
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resourceslinking_interface
)

target_sources(PlasmaActivitiesStats PRIVATE
   ${PlasmaActivitiesStats_DBus_SRCS}
)

target_link_libraries(PlasmaActivitiesStats
   PUBLIC
      Qt6::Core
   PRIVATE
      Qt6::DBus
      Qt6::Sql
      Plasma::Activities
      KF6::ConfigCore
      Threads::Threads
)

target_include_directories(PlasmaActivitiesStats
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/PlasmaActivitiesStats>"
)

# install
ecm_generate_export_header (PlasmaActivitiesStats
   BASE_NAME PlasmaActivitiesStats
   VERSION ${PROJECT_VERSION}
   USE_VERSION_HEADER plasmaactivitiesstats_version.h
   DEPRECATED_BASE_VERSION 0
)

ecm_generate_headers (
   PlasmaActivitiesStats_CamelCase_HEADERS
   HEADER_NAMES
   Query
   Terms
   ResultSet
   ResultWatcher
   ResultModel
   Cleaning

   PREFIX PlasmaActivitiesStats
   REQUIRED_HEADERS PlasmaActivitiesStats_HEADERS
)

install (
   TARGETS PlasmaActivitiesStats
   EXPORT PlasmaActivitiesStatsLibraryTargets
   ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
   )

install (
   FILES ${PlasmaActivitiesStats_CamelCase_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivitiesStats/PlasmaActivities/Stats
   COMPONENT Devel
   )

install (
   FILES ${PlasmaActivitiesStats_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/plasmaactivitiesstats_export.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivitiesStats/plasmaactivitiesstats
   COMPONENT Devel
   )

ecm_qt_install_logging_categories(
    EXPORT PLASMA_ACTIVITIES_STATS
    FILE plasma-activities-stats.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        PlasmaActivitiesStats_QCH
        NAME PlasmaActivitiesStats
        BASE_NAME PlasmaActivitiesStats
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${PlasmaActivitiesStats_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            PlasmaActivities_QCH # while not in API symbols, some classes are referenced in comments
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            PLASMAACTIVITIESSTATS_EXPORT
            PLASMAACTIVITIESSTATS_DEPRECATED
            PLASMAACTIVITIESSTATS_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME PlasmaActivitiesStats
      LIB_NAME PlasmaActivitiesStats
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivitiesStats
      DEPS Qt6Core
      DESCRIPTION "libPlasmaActivitiesStats is a C++ library for using Plasma activities"
      INSTALL
    )
endif ()

