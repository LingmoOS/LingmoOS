cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(ThreadWeaver VERSION ${KF_VERSION})

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
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)
include(ECMAddQch)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(threadweaver_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/threadweaver_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX THREADWEAVER
                        VERSION_HEADER "${threadweaver_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ThreadWeaverConfigVersion.cmake"
                        SOVERSION 6)
# --
ecm_set_disabled_deprecation_versions(
    QT 6.5
)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(benchmarks)
endif()
add_subdirectory(examples)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6ThreadWeaver")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6ThreadWeaver_QCH
        FILE KF6ThreadWeaverQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ThreadWeaverQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6ThreadWeaverConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6ThreadWeaverConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES ${threadweaver_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/ThreadWeaver COMPONENT Devel)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ThreadWeaverConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ThreadWeaverConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6ThreadWeaverTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6ThreadWeaverTargets.cmake NAMESPACE KF6:: )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
