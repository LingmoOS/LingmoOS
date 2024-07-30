cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts

project(KQuickCharts VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0 NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

include(ECMGenerateExportHeader)

include(ECMAddTests)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMQmlModule)
include(ECMDeprecationSettings)

option(BUILD_EXAMPLES "Build example applications" OFF)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Qml Quick QuickControls2 ShaderTools)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

ecm_set_disabled_deprecation_versions(
    QT 6.2.0
)

ecm_setup_version(
    PROJECT
    VARIABLE_PREFIX KF6QuickCharts
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/quickcharts_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6QuickChartsConfigVersion.cmake"
    SOVERSION 1
)

add_subdirectory(controls)
add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

configure_package_config_file(
    "KF6QuickChartsConfig.cmake.in"
    "KF6QuickChartsConfig.cmake"
    INSTALL_DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6QuickCharts
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6QuickChartsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6QuickChartsConfigVersion.cmake"
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6QuickCharts
    COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
