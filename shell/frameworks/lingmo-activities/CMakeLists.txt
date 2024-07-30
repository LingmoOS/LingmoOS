# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "6.1.80")
project(LingmoActivities VERSION ${PROJECT_VERSION})
set(PROJECT_VERSION_MAJOR "6")

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

option (LINGMO_ACTIVITIES_LIBRARY_ONLY "If true, compiles only the LingmoActivities library, without the QML imports." OFF)
option (LINGMO_ACTIVITIES_ENABLE_EXCEPTIONS "If you have Boost 1.53, you need to build LingmoActivities with exceptions enabled. This is UNTESTED and EXPERIMENTAL!" OFF)

set (LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

set(lingmo_activities_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/lib/lingmo_activities_version.h")
ecm_setup_version (
   PROJECT
   VARIABLE_PREFIX LINGMO_ACTIVITIES
   VERSION_HEADER "${lingmo_activities_version_header}"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/LingmoActivitiesConfigVersion.cmake"
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

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/LingmoActivities")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS LingmoActivities_QCH
        FILE LingmoActivitiesLibraryQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/LingmoActivitiesLibraryQchTargets.cmake\")")
endif()

install (
   EXPORT LingmoActivitiesLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE LingmoActivitiesLibraryTargets.cmake
   NAMESPACE Lingmo::
   )

configure_package_config_file (
   "${CMAKE_CURRENT_SOURCE_DIR}/LingmoActivitiesConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/LingmoActivitiesConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   )

install (
   FILES "${CMAKE_CURRENT_BINARY_DIR}/LingmoActivitiesConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/LingmoActivitiesConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )

install (
   FILES ${lingmo_activities_version_header}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/LingmoActivities COMPONENT Devel
   )

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
