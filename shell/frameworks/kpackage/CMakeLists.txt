cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KPackage VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kpackage_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kpackage/kpackage_version.h")
ecm_setup_version(PROJECT
                  VARIABLE_PREFIX PACKAGE
                  VERSION_HEADER "${kpackage_version_header}"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6PackageConfigVersion.cmake"
                  SOVERSION 6)

################# now find all used packages #################

set (REQUIRED_QT_VERSION 6.5.0)

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
  find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
  set(HAVE_DBUS ON)
endif()

find_package(KF6Archive ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)

find_package(KF6DocTools ${KF_DEP_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION "Tools to generate documentation"
                       TYPE OPTIONAL
                      )

ecm_set_disabled_deprecation_versions(
    QT 6.7
    KF 5.239
)

include(KF6PackageMacros.cmake)

# make package_version.h available
include_directories(${CMAKE_CURRENT_BINARY_DIR})

################# list the subdirectories #################
if (KF6DocTools_FOUND)
    add_subdirectory(docs)
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libkpackage6\")
ki18n_install(po)
if (KF6DocTools_FOUND)
    kdoctools_install(po)
endif()
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

################ create PackageConfig.cmake and install it ####################

# create a Config.cmake and a ConfigVersion.cmake file and install them

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Package")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Package_QCH
        FILE KF6PackageQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6PackageQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6PackageConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PackageConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  KDE_INSTALL_INCLUDEDIR_KF CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PackageConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PackageConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6PackageMacros.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6PackageTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6PackageTargets.cmake
        NAMESPACE KF6::
        COMPONENT Devel)
install(EXPORT KF6PackageToolsTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6PackageToolsTargets.cmake
        NAMESPACE KF6::
        COMPONENT Devel)

install(FILES ${kpackage_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPackage COMPONENT Devel )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
