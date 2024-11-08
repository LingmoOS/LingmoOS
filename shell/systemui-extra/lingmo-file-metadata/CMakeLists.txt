cmake_minimum_required(VERSION 3.14)
project(lingmo-file-metadata LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
include(GNUInstallDirs)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckStructHasMember)

#Define QT_MAJOR_VERSION
if (DEFINED QT_MAJOR_VERSION)
    return()
endif()

if (TARGET Qt5::Core)
    set(QT_MAJOR_VERSION 5)
elseif (TARGET Qt6::Core)
    set(QT_MAJOR_VERSION 6)
else()
    option(BUILD_WITH_QT6 "Build against Qt 6" OFF)

    if (BUILD_WITH_QT6)
        set(QT_MAJOR_VERSION 6)
    else()
        set(QT_MAJOR_VERSION 5)
    endif()
endif()

# Dependencies
set(REQUIRED_QT_VERSION 5.12.8)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(POPPLER REQUIRED poppler-qt5)
pkg_search_module(UCHARDET REQUIRED IMPORTED_TARGET uchardet)
pkg_search_module(AVCODEC REQUIRED libavcodec)
pkg_search_module(AVFORMAT REQUIRED libavformat)
pkg_search_module(AVUTIL REQUIRED libavutil)
pkg_search_module(SWSCALE REQUIRED libswscale)
pkg_search_module(TAGLIB REQUIRED taglib)

enable_testing()
add_subdirectory(src)
if (BUILD_TESTING)
add_subdirectory(autotests)
add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

