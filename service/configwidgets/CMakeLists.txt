cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.103.0") # handled by release scripts
set(KF_DEP_VERSION "5.103.0") # handled by release scripts
project(KConfigWidgets VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 5.103.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(CMakeDependentOption)

set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
if (NOT ANDROID)
  find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

set(_qt6_core5_compat FALSE)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat CONFIG REQUIRED) # QTextCodec
    set(_qt6_core5_compat ${Qt6Core5Compat_FOUND})
endif()

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

ecm_setup_version(PROJECT VARIABLE_PREFIX KCONFIGWIDGETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigWidgetsConfigVersion.cmake"
                        SOVERSION 5)
find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Codecs ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF5DocTools ${KF_DEP_VERSION})
find_package(KF5GuiAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF_DEP_VERSION} REQUIRED)

if (NOT ANDROID AND NOT WIN32)
    option(WITH_KAUTH "Build with KAuth support" ON)
else()
    set(WITH_KAUTH OFF)
endif()

set(HAVE_KAUTH FALSE)
if (WITH_KAUTH)
  find_package(KF5Auth ${KF_DEP_VERSION} REQUIRED)
  set(HAVE_KAUTH TRUE)
endif()

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.97.0
    KWIDGETSADDONS 5.77.0  # We need KSelectAction::triggered(QString)
)

add_definitions(-DTRANSLATION_DOMAIN=\"kconfigwidgets5\")
ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    file(GLOB lang_dirs "po/*")
    foreach(lang_dir ${lang_dirs})
       get_filename_component(lang ${lang_dir} NAME)
       if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}/kf5_entry.desktop")
          install( FILES po/${lang}/kf5_entry.desktop DESTINATION ${KDE_INSTALL_LOCALEDIR}/${lang} )
       endif()
    endforeach()
endif()

if (KF5DocTools_FOUND)
    add_subdirectory(docs)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5ConfigWidgets")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5ConfigWidgets_QCH
        FILE KF5ConfigWidgetsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5ConfigWidgetsQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5ConfigWidgetsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigWidgetsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigWidgetsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigWidgetsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5ConfigWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5ConfigWidgetsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigWidgets COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
