cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KService VERSION ${KF_VERSION})

# Disallow in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "KService requires an out of source build. Please create a separate build directory and run 'cmake path_to_kservice [options]' there.")
endif()

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kservice_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kservice_version.h")
ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KSERVICE
                  VERSION_HEADER "${kservice_version_header}"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ServiceConfigVersion.cmake"
                  SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml)

find_package(KF6Config ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6DocTools ${KF_DEP_VERSION} CONFIG)

add_definitions(-DTRANSLATION_DOMAIN=\"kservice6\")

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.102.0
)

ki18n_install(po)

if (KF6DocTools_FOUND)
    kdoctools_install(po)
endif()

if (KF6DocTools_FOUND)
    add_subdirectory(docs)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Service")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Service_QCH
        FILE KF6ServiceQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ServiceQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6ServiceConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6ServiceConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                             )

install(EXPORT KF6ServiceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6ServiceTargets.cmake NAMESPACE KF6:: )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KF6ServiceConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KF6ServiceConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

install(FILES ${kservice_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KService COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
