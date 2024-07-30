# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

add_library(LingmoActivitiesStats)
add_library(Lingmo::ActivitiesStats ALIAS LingmoActivitiesStats)

set_target_properties(LingmoActivitiesStats PROPERTIES
   VERSION     ${LINGMOACTIVITIESSTATS_VERSION}
   SOVERSION   ${LINGMOACTIVITIESSTATS_SOVERSION}
   EXPORT_NAME ActivitiesStats
)

target_sources(LingmoActivitiesStats PRIVATE
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

ecm_qt_declare_logging_category(LingmoActivitiesStats
    HEADER lingmo-activities-stats-logsettings.h
    IDENTIFIER LINGMO_ACTIVITIES_STATS_LOG
    CATEGORY_NAME kde.lingmo.activitiesstats
    OLD_CATEGORY_NAMES kf5.kactivity.stat kf.activitiesstats
    DESCRIPTION "Lingmo Activities Stats"
    EXPORT LINGMO_ACTIVITIES_STATS
)

set(LingmoActivitiesStats_DBus_SRCS)
qt_add_dbus_interface(LingmoActivitiesStats_DBus_SRCS
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesScoring.xml
   resourcesscoring_interface
)
qt_add_dbus_interface(LingmoActivitiesStats_DBus_SRCS
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resourceslinking_interface
)

target_sources(LingmoActivitiesStats PRIVATE
   ${LingmoActivitiesStats_DBus_SRCS}
)

target_link_libraries(LingmoActivitiesStats
   PUBLIC
      Qt6::Core
   PRIVATE
      Qt6::DBus
      Qt6::Sql
      Lingmo::Activities
      KF6::ConfigCore
      Threads::Threads
)

target_include_directories(LingmoActivitiesStats
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/LingmoActivitiesStats>"
)

# install
ecm_generate_export_header (LingmoActivitiesStats
   BASE_NAME LingmoActivitiesStats
   VERSION ${PROJECT_VERSION}
   USE_VERSION_HEADER lingmoactivitiesstats_version.h
   DEPRECATED_BASE_VERSION 0
)

ecm_generate_headers (
   LingmoActivitiesStats_CamelCase_HEADERS
   HEADER_NAMES
   Query
   Terms
   ResultSet
   ResultWatcher
   ResultModel
   Cleaning

   PREFIX LingmoActivitiesStats
   REQUIRED_HEADERS LingmoActivitiesStats_HEADERS
)

install (
   TARGETS LingmoActivitiesStats
   EXPORT LingmoActivitiesStatsLibraryTargets
   ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
   )

install (
   FILES ${LingmoActivitiesStats_CamelCase_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/LingmoActivitiesStats/LingmoActivities/Stats
   COMPONENT Devel
   )

install (
   FILES ${LingmoActivitiesStats_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/lingmoactivitiesstats_export.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/LingmoActivitiesStats/lingmoactivitiesstats
   COMPONENT Devel
   )

ecm_qt_install_logging_categories(
    EXPORT LINGMO_ACTIVITIES_STATS
    FILE lingmo-activities-stats.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        LingmoActivitiesStats_QCH
        NAME LingmoActivitiesStats
        BASE_NAME LingmoActivitiesStats
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${LingmoActivitiesStats_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            LingmoActivities_QCH # while not in API symbols, some classes are referenced in comments
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            LINGMOACTIVITIESSTATS_EXPORT
            LINGMOACTIVITIESSTATS_DEPRECATED
            LINGMOACTIVITIESSTATS_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME LingmoActivitiesStats
      LIB_NAME LingmoActivitiesStats
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/LingmoActivitiesStats
      DEPS Qt6Core
      DESCRIPTION "libLingmoActivitiesStats is a C++ library for using Lingmo activities"
      INSTALL
    )
endif ()

