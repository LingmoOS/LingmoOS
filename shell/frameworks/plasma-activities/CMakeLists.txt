# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "6.1.80")
project(PlasmaActivities VERSION ${PROJECT_VERSION})
set(PROJECT_VERSION_MAJOR "6")

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

option (PLASMA_ACTIVITIES_LIBRARY_ONLY "If true, compiles only the PlasmaActivities library, without the QML imports." OFF)
option (PLASMA_ACTIVITIES_ENABLE_EXCEPTIONS "If you have Boost 1.53, you need to build PlasmaActivities with exceptions enabled. This is UNTESTED and EXPERIMENTAL!" OFF)

set (PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(ECMMarkNonGuiExecutable)
include(ECMDeprecationSettings)
include(ECMQmlModule)
include(KDEClangFormat)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

find_package (Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core DBus)

include (CMakePackageConfigHelpers)
include (ECMSetupVersion)

set(plasma_activities_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/lib/plasma_activities_version.h")
ecm_setup_version (
   PROJECT
   VARIABLE_PREFIX PLASMA_ACTIVITIES
   VERSION_HEADER "${plasma_activities_version_header}"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesConfigVersion.cmake"
   SOVERSION 6
   )

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.102.0
)

add_subdirectory (src)
if (BUILD_TESTING)
    add_subdirectory (autotests)
    add_subdirectory (tests)
endif()

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/PlasmaActivities")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS PlasmaActivities_QCH
        FILE PlasmaActivitiesLibraryQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/PlasmaActivitiesLibraryQchTargets.cmake\")")
endif()

install (
   EXPORT PlasmaActivitiesLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE PlasmaActivitiesLibraryTargets.cmake
   NAMESPACE Plasma::
   )

configure_package_config_file (
   "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaActivitiesConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   )

install (
   FILES "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/PlasmaActivitiesConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )

install (
   FILES ${plasma_activities_version_header}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/PlasmaActivities COMPONENT Devel
   )

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
