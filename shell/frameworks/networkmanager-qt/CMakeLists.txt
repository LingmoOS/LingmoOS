cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(NetworkManagerQt VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

find_package(PkgConfig)
pkg_search_module(NetworkManager REQUIRED IMPORTED_TARGET libnm>=1.4.0)
pkg_search_module(Gio REQUIRED IMPORTED_TARGET gio-2.0)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED Core DBus Network Qml)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMQmlModule)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT VARIABLE_PREFIX NETWORKMANAGERQT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/networkmanagerqt_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6NetworkManagerQtConfigVersion.cmake"
                        SOVERSION 6)
ecm_set_disabled_deprecation_versions(
    QT 6.5
)

# Enable unit testing
if (BUILD_TESTING)
    add_subdirectory(src/fakenetwork)
    add_subdirectory(autotests)
endif ()

add_subdirectory(src)
##################  examples  ################################
option(BUILD_EXAMPLES "Build example applications" FALSE)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_EXAMPLES)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6NetworkManagerQt")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6NetworkManagerQt_QCH
        FILE KF6NetworkManagerQtQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6NetworkManagerQtQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6NetworkManagerQtConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NetworkManagerQtConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NetworkManagerQtConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NetworkManagerQtConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6NetworkManagerQtTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6NetworkManagerQtTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/networkmanagerqt_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/NetworkManagerQt COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
