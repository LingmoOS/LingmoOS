cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KTextTemplate VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  CONFIG)
set_package_properties(ECM PROPERTIES
  TYPE REQUIRED
  DESCRIPTION "Extra CMake Modules."
  URL "https://commits.kde.org/extra-cmake-modules"
)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt6Qml ${REQUIRED_QT_VERSION} NO_MODULE)

set_package_properties(Qt6Qml PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Build Javascript binding for KTextDocument"
)

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMSetupVersion)
include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(KTextTemplate_MODULE_DIR ${CMAKE_SOURCE_DIR}/cmake/modules)
list(APPEND CMAKE_MODULE_PATH ${KTextTemplate_MODULE_DIR})

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wshadow -std=iso9899:1990 -Werror-implicit-function-declaration")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -fno-check-new")
endif()

ecm_set_disabled_deprecation_versions(
    QT 6.5
)

kde_enable_exceptions()

set( KTextTemplate_INSTALL_PLUGINDIR ${KDE_INSTALL_PLUGINDIR}/kf6/ktexttemplate )

include("${KTextTemplate_MODULE_DIR}/KF6TextTemplateMacros.cmake")

set(ktexttemplate_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/lib/ktexttemplate_version.h")
ecm_setup_version(PROJECT
  VARIABLE_PREFIX KTEXTTEMPLATE
  VERSION_HEADER "${ktexttemplate_version_header}"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6TextTemplateConfigVersion.cmake"
  SOVERSION 6
)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6TextTemplate" )

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6TextTemplate_QCH
        FILE KF6TextTemplateQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6TextTemplateQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${KTextTemplate_MODULE_DIR}/KF6TextTemplateConfig.cmake.in"
  "${CMAKE_BINARY_DIR}/KF6TextTemplateConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_BINARY_DIR}/KF6TextTemplateConfig.cmake"
  "${CMAKE_BINARY_DIR}/KF6TextTemplateConfigVersion.cmake"
  "${KTextTemplate_MODULE_DIR}/KF6TextTemplateMacros.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6TextTemplateTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  NAMESPACE KF6::
  FILE KF6TextTemplateTargets.cmake
)

install(FILES
  ${ktexttemplate_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KTextTemplate
  COMPONENT Devel
)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
