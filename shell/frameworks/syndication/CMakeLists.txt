cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts

project(Syndication VERSION ${KF_VERSION})

set(REQUIRED_QT_VERSION 6.5.0)

# ECM setup
find_package(ECM ${KF_DEP_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(ECMDeprecationSettings)

set(syndication_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/syndication_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX SYNDICATION
                        VERSION_HEADER "${syndication_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6SyndicationConfigVersion.cmake"
                        SOVERSION 6
)

########### Find packages ###########
find_package(Qt6Xml ${REQUIRED_QT_VERSION} CONFIG REQUIRED)

find_package(KF6Codecs ${KF_DEP_VERSION} CONFIG REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240
)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

########### Targets ###########
if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif()
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Syndication")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Syndication_QCH
        FILE KF6SyndicationQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6SyndicationQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6SyndicationConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6SyndicationConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6SyndicationConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6SyndicationConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6SyndicationTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6SyndicationTargets.cmake NAMESPACE KF6::)

install(FILES
   ${syndication_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Syndication COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
