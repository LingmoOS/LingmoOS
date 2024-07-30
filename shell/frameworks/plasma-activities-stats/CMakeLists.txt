# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)
project(PlasmaActivitiesStats VERSION ${PROJECT_VERSION})

set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(KASTATS_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core DBus Sql)

find_package(KF6Config ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package(PlasmaActivities ${PROJECT_DEP_VERSION} CONFIG REQUIRED)
find_package(Threads REQUIRED)

include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

set(plasmaactivitiesstats_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/plasmaactivitiesstats_version.h")
ecm_setup_version (
   PROJECT
   VARIABLE_PREFIX PLASMAACTIVITIESSTATS
   VERSION_HEADER ${plasmaactivitiesstats_version_header}
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesStatsConfigVersion.cmake"
   SOVERSION 1
   )
ecm_set_disabled_deprecation_versions(
    QT 6.4.0
    KF 5.102.0
)

add_subdirectory(src)
if(BUILD_TESTING)
  set(Boost_NO_BOOST_CMAKE ON)
  find_package(Boost 1.49)
  if (Boost_FOUND)
    add_subdirectory(autotests)
  endif()
  add_subdirectory(tests)
endif(BUILD_TESTING)

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/PlasmaActivitiesStats")

if(BUILD_QCH)
    ecm_install_qch_export(
        TARGETS PlasmaActivitiesStats_QCH
        FILE PlasmaActivitiesStatsLibraryQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/PlasmaActivitiesStatsLibraryQchTargets.cmake\")")
endif()

install(
   EXPORT PlasmaActivitiesStatsLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE PlasmaActivitiesStatsLibraryTargets.cmake
   NAMESPACE Plasma::
   )

configure_package_config_file(
   "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaActivitiesStatsConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesStatsConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   )

install(
   FILES "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesStatsConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesStatsConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )

install(
   FILES ${plasmaactivitiesstats_version_header}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivitiesStats COMPONENT Devel
   )

# Write out the features
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
