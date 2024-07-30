# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KNewStuff VERSION ${KF_VERSION})

set(CMAKE_CXX_EXTENSIONS OFF)

include(FeatureSummary)
find_package(ECM 6.4.0 NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

include(ECMQmlModule)
include(ECMQtDeclareLoggingCategory)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS Gui Widgets Xml Qml Quick QuickWidgets)
find_package(KF6Archive ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Package ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMDeprecationSettings)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(CMakeDependentOption)

find_package(KF6Attica ${KF_DEP_VERSION} CONFIG REQUIRED)
set_package_properties(KF6Attica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in KNewStuff"
                       URL "https://commits.kde.org/attica"
                       TYPE REQUIRED
                      )
find_package(KF6Kirigami2 2.12.0)
set_package_properties(KF6Kirigami2 PROPERTIES
    DESCRIPTION "KDE's lightweight user interface framework for mobile and convergent applications"
    URL "https://techbase.kde.org/Kirigami"
    PURPOSE "Required by KNewStuffQuick's qml components"
    TYPE RUNTIME)

find_package(KF6Syndication ${KF6_DEP_VERSION})
set_package_properties(KF6Syndication PROPERTIES
    DESCRIPTION "KDE's RSS/Atom parser library"
    PURPOSE "Used by the OPDS provider to handle most of the parsing."
    TYPE OPTIONAL
)


option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

ecm_setup_version(PROJECT VARIABLE_PREFIX KNEWSTUFF
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffConfigVersion.cmake"
                        SOVERSION 6)
ecm_setup_version(PROJECT VARIABLE_PREFIX KNEWSTUFFCORE
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffCoreConfigVersion.cmake"
                        SOVERSION 6)
ecm_setup_version(PROJECT VARIABLE_PREFIX KNEWSTUFFWIDGETS
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffWidgetsConfigVersion.cmake"
                        SOVERSION 6)

add_definitions(-DTRANSLATION_DOMAIN=\"knewstuff6\")
ecm_set_disabled_deprecation_versions(
    QT 6.5.0
    KF 5.102
)

ki18n_install(po)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6NewStuff")
set(CMAKECONFIG_CORE_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6NewStuffCore")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6NewStuff_QCH
        FILE KF6NewStuffQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6NewStuffQchTargets.cmake\")")
    ecm_install_qch_export(
        TARGETS KF6NewStuffCore_QCH
        FILE KF6NewStuffCoreQchTargets.cmake
        DESTINATION "${CMAKECONFIG_CORE_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_CORE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6NewStuffCoreQchTargets.cmake\")")
    # TODO: KF6NewStuffQuick
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6NewStuffConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6NewStuffCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffCoreConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_CORE_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffCoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NewStuffCoreConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_CORE_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6NewStuffTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6NewStuffTargets.cmake NAMESPACE KF6::)
install(EXPORT KF6NewStuffCoreTargets DESTINATION "${CMAKECONFIG_CORE_INSTALL_DIR}" FILE KF6NewStuffCoreTargets.cmake NAMESPACE KF6::)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff
  COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
