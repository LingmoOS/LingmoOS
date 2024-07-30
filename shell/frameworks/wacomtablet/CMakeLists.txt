cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "6.1.80")
project(wacomtablet VERSION ${PROJECT_VERSION})

set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX WACOMTABLET
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/wacomtablet-version.h"
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Qml)
find_package(KF6 REQUIRED COMPONENTS CoreAddons I18n GlobalAccel Config XmlGui WidgetsAddons WindowSystem Notifications DBusAddons DocTools KCMUtils KIO)
find_package(XCB REQUIRED COMPONENTS XINPUT)
find_package(X11 REQUIRED)

find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)
find_package(Plasma5Support REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBWACOM libwacom REQUIRED IMPORTED_TARGET)
pkg_check_modules(XORGWACOM xorg-wacom REQUIRED IMPORTED_TARGET)

if(${LIBWACOM_VERSION} VERSION_LESS "0.29")
    message(STATUS "Button detection with libwacom requires version at least 0.29. Detected version is: " ${LIBWACOM_VERSION})
    add_definitions(-DLIBWACOM_EVDEV_MISSING)
endif()

add_definitions(-DQT_STRICT_ITERATORS)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_USE_QSTRINGBUILDER)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(images)

ki18n_install(po)
if(KF6DocTools_FOUND)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

### Tests
if(BUILD_TESTING)
  find_package(Qt6Test ${QT_MIN_VERSION} CONFIG REQUIRED)
  add_subdirectory(autotests)
endif()
ecm_qt_install_logging_categories(
        EXPORT WACOMTABLET
        FILE wacomtablet.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
