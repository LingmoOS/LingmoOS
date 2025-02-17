cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.103.0") # handled by release scripts
set(KF_DEP_VERSION "5.103.0") # handled by release scripts
project(KJobWidgets VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 5.103.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)
include(ECMPoQmTools)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)
include(ECMAddQch)
include(CMakePackageConfigHelpers)

set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

if (NOT ANDROID)
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()
if (NOT APPLE)
    find_package(X11)
endif()

set(HAVE_X11 ${X11_FOUND})

if (HAVE_X11)
    if (QT_MAJOR_VERSION STREQUAL "5")
        find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras)
    else()
        # qtx11extras_p.h is in Qt6Gui, which implied by the Qt6Widgets above
    endif()
endif()

find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF_DEP_VERSION} REQUIRED)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT VARIABLE_PREFIX KJOBWIDGETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kjobwidgets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5JobWidgetsConfigVersion.cmake"
                        SOVERSION 5)

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.95
)

ecm_install_po_files_as_qm(poqm)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5JobWidgets")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5JobWidgets_QCH
        FILE KF5JobWidgetsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5JobWidgetsQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5JobWidgetsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5JobWidgetsConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5JobWidgetsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5JobWidgetsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5JobWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5JobWidgetsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kjobwidgets_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KJobWidgets COMPONENT Devel
)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
