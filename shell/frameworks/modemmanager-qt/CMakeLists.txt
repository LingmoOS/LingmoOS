cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(ModemManagerQt VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

find_package(ModemManager 1.0 REQUIRED)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED Core DBus Xml)

add_definitions(${QT_DEFINITIONS})

remove_definitions(-DQT_NO_FOREACH) # set by KDEFrameworkCompilerSettings

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(modemmanagerqt_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/modemmanagerqt_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX MODEMMANAGERQT
                        VERSION_HEADER "${modemmanagerqt_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ModemManagerQtConfigVersion.cmake"
                        SOVERSION 6)
ecm_set_disabled_deprecation_versions(
    QT 6.5
)

# Enable unit testing
if (BUILD_TESTING)
    add_subdirectory(src/fakemodem)
    add_subdirectory(autotests)
endif ()

##################  examples  ################################
option(BUILD_EXAMPLES "Build example applications" FALSE)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_EXAMPLES)

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6ModemManagerQt")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6ModemManagerQt_QCH
        FILE KF6ModemManagerQtQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ModemManagerQtQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6ModemManagerQtConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ModemManagerQtConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ModemManagerQtConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ModemManagerQtConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6ModemManagerQtTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6ModemManagerQtTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${modemmanagerqt_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/ModemManagerQt COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
