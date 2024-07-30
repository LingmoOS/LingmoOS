cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KNotifyConfig VERSION ${KF_VERSION})

# Dependencies
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# Includes
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(CMakePackageConfigHelpers)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

# Required Qt components to build this framework
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED DBus)
    set(HAVE_DBUS TRUE)
    add_definitions(-DHAVE_DBUS)
endif()

find_package(KF6Completion ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6KIO ${KF_DEP_VERSION} REQUIRED)

find_package(Canberra)
set_package_properties(Canberra PROPERTIES
    PURPOSE "Needed to preview notification sounds"
    TYPE OPTIONAL)
if (NOT Canberra_FOUND)
    # This is REQUIRED since you cannot tell CMake "either one of those two optional ones are required"
    find_package(Phonon4Qt6 4.6.60 NO_MODULE REQUIRED)
    set_package_properties(Phonon4Qt6 PROPERTIES
        DESCRIPTION "Qt-based audio library"
        PURPOSE "Needed to preview notification sounds when Canberra isn't available")
endif()

# ECM setup
set(knotifyconfig_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/knotifyconfig_version.h")
ecm_setup_version(
  PROJECT
  VARIABLE_PREFIX KNOTIFYCONFIG
  VERSION_HEADER "${knotifyconfig_version_header}"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6NotifyConfigConfigVersion.cmake"
  SOVERSION 6)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240
)

# Subdirectories
add_definitions(-DTRANSLATION_DOMAIN=\"knotifyconfig6\")
ki18n_install(po)
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6NotifyConfig")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6NotifyConfig_QCH
        FILE KF6NotifyConfigQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6NotifyConfigQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6NotifyConfigConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NotifyConfigConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NotifyConfigConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NotifyConfigConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KF6NotifyConfigTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF6NotifyConfigTargets.cmake
  NAMESPACE KF6::
)

install(FILES
  "${knotifyconfig_version_header}"
  DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}/KNotifyConfig"
  COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
