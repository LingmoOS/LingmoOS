cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(Attica VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDEGitCommitHooks)

include(ECMGenerateExportHeader)
include(ECMGeneratePkgConfigFile)
include(ECMCheckOutboundLicense)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers) # Used to create CMake config files
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(attica_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/attica_version.h")
ecm_setup_version(PROJECT
    VARIABLE_PREFIX ATTICA
    VERSION_HEADER "${attica_version_header}"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6AtticaConfigVersion.cmake"
    SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)

# Required Qt components to build this framework
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Network)

set(ATTICA_LIB_SONAME KF6Attica)

# Enable static build
option(ATTICA_STATIC_BUILD "Build a static library" Off)

ecm_set_disabled_deprecation_versions(
    QT 6.5
)

add_subdirectory(src)

# Enable unit testing
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif ()

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Attica")

# Create the CMake Config files
if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Attica_QCH
        FILE KF6AtticaQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6AtticaQchTargets.cmake\")")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6AtticaConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6AtticaConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/KF6AtticaConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/KF6AtticaConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KF6AtticaTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6AtticaTargets.cmake
        NAMESPACE KF6::)

install(FILES "${attica_version_header}"
        DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}/Attica"
        COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
