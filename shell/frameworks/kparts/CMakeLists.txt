cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KParts VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEPackageAppTemplates)
include(ECMAddQch)

include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMDeprecationSettings)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control how much of deprecated API is build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kparts_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kparts_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KPARTS
                        VERSION_HEADER "${kparts_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6PartsConfigVersion.cmake"
                        SOVERSION 6)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Widgets Xml)

find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6KIO ${KF_DEP_VERSION} REQUIRED)
find_package(KF6JobWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Service ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6XmlGui ${KF_DEP_VERSION} REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240.0
)

if(BUILD_TESTING)
  add_subdirectory( tests )
  add_subdirectory( autotests )
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"kparts6\")
ki18n_install(po)

add_subdirectory( src )
add_subdirectory(templates)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Parts")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Parts_QCH
        FILE KF6PartsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6PartsQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6PartsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PartsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PartsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PartsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6PartsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6PartsTargets.cmake NAMESPACE KF6:: )

install(FILES
  ${kparts_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KParts COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
