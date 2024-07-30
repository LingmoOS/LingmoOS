cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(FrameworkIntegration VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})


include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui Widgets)


ecm_setup_version(PROJECT VARIABLE_PREFIX FRAMEWORKINTEGRATION
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6FrameworkIntegrationConfigVersion.cmake"
                  SOVERSION 6)

find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ColorScheme ${KF_DEP_VERSION} REQUIRED)
find_package(KF6IconThemes ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Notifications ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)


option(BUILD_KPACKAGE_INSTALL_HANDLERS "Build the KPackage install handler binaries (recommended)" ON)
if (BUILD_KPACKAGE_INSTALL_HANDLERS)
   find_package(KF6NewStuffCore ${KF_DEP_VERSION} REQUIRED)
   find_package(KF6Package ${KF_DEP_VERSION} REQUIRED)
   find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)

   find_package(packagekitqt6)
   find_package(AppStreamQt 1.0)
endif()

ecm_set_disabled_deprecation_versions(
    QT 6.5.0
    KF 5.102
)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6FrameworkIntegration")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6FrameworkIntegrationConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6FrameworkIntegrationConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6FrameworkIntegrationConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6FrameworkIntegrationConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF6FrameworkIntegrationTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6FrameworkIntegrationTargets.cmake NAMESPACE KF6:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/FrameworkIntegration COMPONENT Devel)

install(FILES lingmo_workspace.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})


include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
