cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
set(REQUIRED_QT_VERSION 6.5.0)
project(KDeclarative VERSION ${KF_VERSION})

# Dependencies
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQmlModule)
include(CMakePackageConfigHelpers)
include(ECMAddQch)

find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Qml Quick Gui)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6GuiAddons ${KF_DEP_VERSION} REQUIRED)


if(NOT WIN32 AND NOT APPLE AND NOT ANDROID)
    find_package(KF6GlobalAccel ${KF_DEP_VERSION} REQUIRED)
    set(HAVE_KGLOBALACCEL TRUE)
else()
    set(HAVE_KGLOBALACCEL FALSE)
endif()

if (NOT ANDROID)
  find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
endif()

configure_file(config-kdeclarative.h.in ${CMAKE_BINARY_DIR}/config-kdeclarative.h)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_EXAMPLES "Build and install examples." OFF)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")


# ECM setup
ecm_setup_version(
  PROJECT
  VARIABLE_PREFIX KDECLARATIVE
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6DeclarativeConfigVersion.cmake"
  SOVERSION 6)

# Subdirectories
add_definitions(-DTRANSLATION_DOMAIN=\"kdeclarative6\")
ecm_set_disabled_deprecation_versions(
    QT 6.5.0
    KF 5.102
)

ki18n_install(po)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Declarative")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Declarative_QCH
        FILE KF6DeclarativeQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6DeclarativeQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6DeclarativeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6DeclarativeConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6DeclarativeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6DeclarativeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KF6DeclarativeTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF6DeclarativeTargets.cmake
  NAMESPACE KF6::
  COMPONENT Devel
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_version.h"
  DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}/KDeclarative"
  COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
