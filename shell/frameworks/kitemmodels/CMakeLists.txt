cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KItemModels VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0 NO_MODULE)
set_package_properties(ECM PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Extra CMake Modules."
    URL "https://commits.kde.org/extra-cmake-modules"
)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)
include(ECMAddQch)
include(CMakePackageConfigHelpers)
include(ECMQmlModule)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt6Qml ${REQUIRED_QT_VERSION} NO_MODULE)

set_package_properties(Qt6Qml PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Build QML import for KItemModels and proxymodeltestapp"
)


set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kitemmodels_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/core/kitemmodels_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KITEMMODELS
    VERSION_HEADER "${kitemmodels_version_header}"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ItemModelsConfigVersion.cmake"
    SOVERSION 6
)

ecm_set_disabled_deprecation_versions(
    QT 6.5
)

add_subdirectory(src)
if (BUILD_TESTING)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets)
    find_package(Qt6QuickWidgets ${REQUIRED_QT_VERSION} NO_MODULE)
    set_package_properties(Qt6QuickWidgets PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Build proxymodeltestapp QML selection and QML treeview examples"
    )
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6ItemModels")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6ItemModels_QCH
        FILE KF6ItemModelsQCHTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ItemModelsQCHTargets.cmake\")")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6ItemModelsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6ItemModelsConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6ItemModelsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6ItemModelsConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(EXPORT KF6ItemModelsTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF6ItemModelsTargets.cmake
    NAMESPACE KF6::
)

install(FILES
    ${kitemmodels_version_header}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KItemModels
    COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
