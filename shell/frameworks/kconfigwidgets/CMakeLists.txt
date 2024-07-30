cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KConfigWidgets VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
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

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
  find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
  set(HAVE_DBUS ON)
endif()

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

set(kconfigwidgets_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kconfigwidgets_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KCONFIGWIDGETS
                        VERSION_HEADER "${kconfigwidgets_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigWidgetsConfigVersion.cmake"
                        SOVERSION 6)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Codecs ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6GuiAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ColorScheme ${KF_DEP_VERSION} REQUIRED)


ecm_set_disabled_deprecation_versions(
    QT 6.5.0
    KF 5.102.0
)

add_definitions(-DTRANSLATION_DOMAIN=\"kconfigwidgets6\")
ki18n_install(po)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    file(GLOB lang_dirs "po/*")
    foreach(lang_dir ${lang_dirs})
       get_filename_component(lang ${lang_dir} NAME)
       if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}/kf6_entry.desktop")
          install( FILES po/${lang}/kf6_entry.desktop DESTINATION ${KDE_INSTALL_LOCALEDIR}/${lang} )
       endif()
    endforeach()
endif()

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6ConfigWidgets")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6ConfigWidgets_QCH
        FILE KF6ConfigWidgetsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ConfigWidgetsQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6ConfigWidgetsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigWidgetsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigWidgetsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigWidgetsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6ConfigWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6ConfigWidgetsTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${kconfigwidgets_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigWidgets COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
