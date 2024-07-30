cmake_minimum_required(VERSION 3.16)

project(decoration-api)
set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGenerateExportHeader)
include(FeatureSummary)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KDECORATION2
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdecoration2_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDecoration2ConfigVersion.cmake"
                        SOVERSION ${PROJECT_VERSION_MAJOR})

#dependencies
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Test
)

# require at least gcc 4.8
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8")
        message(SEND_ERROR "Version ${CMAKE_CXX_COMPILER_VERSION} of the ${CMAKE_CXX_COMPILER_ID} C++ compiler is not supported. Please use version 4.8 or later.")
    endif()
endif()

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(KDECORATION2_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/KDecoration2")
find_package(KF6I18n ${KF6_MIN_VERSION} CONFIG REQUIRED)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.240.0
    )

add_definitions(-DQT_NO_KEYWORDS)

# Subdirectories
add_subdirectory(src)
if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KDecoration2")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDecoration2Config.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/KDecoration2Config.cmake"
                                  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                                  )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KDecoration2Config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KDecoration2ConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KDecoration2Targets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDecoration2Targets.cmake NAMESPACE KDecoration2:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdecoration2_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF} COMPONENT Devel )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

ki18n_install(po)
