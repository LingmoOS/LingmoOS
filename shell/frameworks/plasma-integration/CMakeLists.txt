cmake_minimum_required(VERSION 3.16)

project(PlasmaIntegration)
set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(QT5_MIN_VERSION "5.15.2")
set(QT_MIN_VERSION "6.6.0")
set(KF5_MIN_VERSION "5.102.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_NO_CREATE_VERSIONLESS_TARGETS ON)
set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS ON)

find_package(ECM ${KF5_MIN_VERSION}  REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} )

# We need this because we can't include KDECMakeSettings here, because that
# would need KDEInstallDirs, which we can only include in the qt{5,6} subdirs
# to get the correct installation dirs for each Qt version.
option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
    enable_testing()
endif()

include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Wayland 1.9 REQUIRED Client)
find_package(PlasmaWaylandProtocols 1.6.0 CONFIG REQUIRED)

option(BUILD_QT5 "Build for Qt5" ON)
option(BUILD_QT6 "Build for Qt6" ON)

#CI is stupid and doesn't allow us to set CMAKE options per build variant
if ($ENV{CI_JOB_NAME_SLUG} MATCHES "qt5")
    set(BUILD_QT5 ON)
    set(BUILD_QT6 OFF)
elseif($ENV{CI_JOB_NAME_SLUG} MATCHES "qt6")
    set(BUILD_QT5 OFF)
    set(BUILD_QT6 ON)
endif()

find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
    DESCRIPTION "X protocol C-language Binding"
    URL "https://xcb.freedesktop.org"
    TYPE REQUIRED
    PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

find_package(FontNotoSans)
set_package_properties(FontNotoSans PROPERTIES
    PURPOSE "Default sans-serif font -- this is not detected automatically, pass -DCMAKE_DISABLE_FIND_PACKAGE_FontNotoSans=true to mark it ignored."
    URL "https://www.google.com/get/noto/"
    TYPE RUNTIME
)
find_package(FontNotoColorEmoji)
set_package_properties(FontNotoColorEmoji PROPERTIES
    PURPOSE "Default emoji font -- this is not detected automatically, pass -DCMAKE_DISABLE_FIND_PACKAGE_FontNotoColorEmoji=true to mark it ignored."
    URL "https://www.google.com/get/noto/"
    TYPE RUNTIME
)
find_package(FontHack)
set_package_properties(FontHack PROPERTIES
    PURPOSE "Default monospace font -- this is not detected automatically, pass -DCMAKE_DISABLE_FIND_PACKAGE_FontHack=true to mark it ignored."
    URL "https://sourcefoundry.org/hack/"
    TYPE RUNTIME
)
find_package(XDGDesktopPortalKDE)
set_package_properties(XDGDesktopPortalKDE PROPERTIES
    PURPOSE "Required for open-with dialog to work correctly -- this is not detected automatically, pass -DCMAKE_DISABLE_FIND_PACKAGE_XDGDesktopPortalKDE=true to mark it ignored."
    URL "https://invent.kde.org/plasma/xdg-desktop-portal-kde"
    TYPE RUNTIME
)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

if (${BUILD_QT5})
    unset(QUERY_EXECUTABLE CACHE)
    add_subdirectory(qt5)
endif()

if (${BUILD_QT6})
    unset(QUERY_EXECUTABLE CACHE)

    add_definitions(-DTRANSLATION_DOMAIN=\"plasmaintegration5\")
    add_subdirectory(qt6)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
