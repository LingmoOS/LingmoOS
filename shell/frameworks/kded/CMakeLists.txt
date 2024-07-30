cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KDED VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION 6.5.0)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt6 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED DBus Widgets)
find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Crash ${KF_DEP_VERSION} REQUIRED)
find_package(KF6DBusAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6DocTools ${KF_DEP_VERSION})
find_package(KF6Service ${KF_DEP_VERSION} REQUIRED)


include(CMakePackageConfigHelpers)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)

ecm_setup_version(
  PROJECT
  VARIABLE_PREFIX KDED
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kded_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6KDEDConfigVersion.cmake"
  SOVERSION 6)

# We don't install kded_version.h, it's only useful internally
# (given that this framework produces no library)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240.0
)

if (KF6DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(docs)
endif()

add_subdirectory(src)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6KDED")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6KDEDConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6KDEDConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6KDEDConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6KDEDConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(FILES org.kde.kded6.desktop DESTINATION ${KDE_INSTALL_APPDIR})

ecm_qt_install_logging_categories(
        EXPORT KDED
        FILE kded.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
