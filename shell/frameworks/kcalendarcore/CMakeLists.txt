cmake_minimum_required(VERSION 3.16)
set(KF_VERSION "6.5.0") # handled by release scripts

project(KCalendarCore VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(REQUIRED_QT_VERSION 6.5.0)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(ECMDeprecationSettings)

include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(ECMQmlModule)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(KCALENDARCORE_LIB_VERSION ${KF_VERSION})
set(kcalendarcore_version_header "${KCalendarCore_BINARY_DIR}/src/kcalendarcore_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KCALENDARCORE
  VERSION_HEADER "${kcalendarcore_version_header}"
  PACKAGE_VERSION_FILE "${KCalendarCore_BINARY_DIR}/KF6CalendarCoreConfigVersion.cmake"
  SOVERSION 6
)

########### Find packages ###########
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui)

set(LibIcal_MIN_VERSION "3.0")
# TODO remove our FindLibIcal.cmake in favor of upstream LibIcalConfig.cmake once the following issues are resolved:
# https://github.com/libical/libical/pull/510
# https://bugs.launchpad.net/ubuntu/+source/libical3/+bug/1946963
# https://bugs.launchpad.net/ubuntu/+source/libical/+bug/1920587
find_package(LibIcal ${LibIcal_MIN_VERSION})
set_package_properties(LibIcal PROPERTIES TYPE REQUIRED)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6CalendarCore")

if (BUILD_QCH)
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6CalendarCoreQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6CalendarCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6CalendarCoreConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

ecm_set_disabled_deprecation_versions(
    QT 6.5
)

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
  find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)
  add_subdirectory(autotests)
endif()

########### Install Files ###########
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6CalendarCoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6CalendarCoreConfigVersion.cmake"
  cmake/FindLibIcal.cmake
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6CalendarCoreTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF6CalendarCoreTargets.cmake
  NAMESPACE KF6::
)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6CalendarCore_QCH
        FILE KF6CalendarCoreQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
endif()

install(FILES
  ${kcalendarcore_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCalendarCore COMPONENT Devel
)

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF6CalendarCore
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KCalendarCore/
      DEPS "Qt6Core Qt6Gui"
    INSTALL)
endif()

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
