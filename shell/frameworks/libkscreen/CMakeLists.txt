cmake_minimum_required(VERSION 3.16)

project(libkscreen)
set(PROJECT_VERSION "6.1.80")

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMSetupVersion)
include(ECMMarkAsTest)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(ECMPoQmTools)
include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Gui Test WaylandClient)
find_package(WaylandScanner)


find_package(PlasmaWaylandProtocols 1.13 CONFIG)
set_package_properties(PlasmaWaylandProtocols PROPERTIES TYPE REQUIRED)

find_package(Wayland 1.15 COMPONENTS Client)
set_package_properties(Wayland PROPERTIES
                       TYPE REQUIRED
                      )

# xrandr backend

find_package(XCB COMPONENTS XCB RANDR DPMS)
set_package_properties(XCB PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Required for building XRandR backends"
)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

find_program(JQ NAMES "jq" DOC "jq is used in zsh completion script")
if(JQ)
  message(STATUS "Found jq executable: ${JQ}")
else()
  message(WARNING "jq executable not found, zsh completions won't work at their full capacity")
endif()

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KSCREEN
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kscreen_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ScreenConfigVersion.cmake"
                        SOVERSION 8)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)

add_subdirectory(src)
add_subdirectory(backends)
if(BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

ecm_install_po_files_as_qm(poqm)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Screen")

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)


if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Screen_QCH
        FILE KF6ScreenQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ScreenQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6ScreenConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6ScreenConfig.cmake"
                              INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ScreenConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ScreenConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KF6ScreenTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF6ScreenTargets.cmake
  NAMESPACE KF6::
  COMPONENT Devel
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kscreen_version.h"
  DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}"
  COMPONENT Devel
)
ecm_qt_install_logging_categories(EXPORT KSCREEN KSCREEN_DPMS KSCREEN_EDID FILE libkscreen.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
