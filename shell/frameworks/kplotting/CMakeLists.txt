cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KPlotting VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)
include(ECMAddQch)
include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

set(kplotting_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kplotting_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KPLOTTING
                        VERSION_HEADER "${kplotting_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6PlottingConfigVersion.cmake"
                        SOVERSION 6)
ecm_set_disabled_deprecation_versions(
    QT 6.5
)

add_subdirectory(src)
add_subdirectory(examples)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Plotting")

if(BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Plotting_QCH
        FILE KF6PlottingQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6PlottingQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6PlottingConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6PlottingConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KF6PlottingConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/KF6PlottingConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KF6PlottingTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6PlottingTargets.cmake NAMESPACE KF6::)

install(FILES ${kplotting_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPlotting COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
