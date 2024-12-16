cmake_minimum_required(VERSION 3.25.0)

project(TreeLand
    VERSION 0.2.2
    LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Standard installation paths
include(GNUInstallDirs)
# Macros
include(FeatureSummary)

option(WITH_SUBMODULE_WAYLIB "Use the waylib from git submodule" ON)
add_feature_info(submodule_waylib WITH_SUBMODULE_WAYLIB "Use waylib from submodule")

option(ADDRESS_SANITIZER "Enable address sanitize" OFF)
add_feature_info(ASanSupport ADDRESS_SANITIZER "https://github.com/google/sanitizers/wiki/AddressSanitizer")

option(BUILD_TEST_CLIENTS "Build clients demo to test treeland" OFF)
add_feature_info(DemoClents BUILD_TEST_CLIENTS "clients demo for testing")

if (ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address -fno-optimize-sibling-calls -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

set(QML_IMPORT_PATH "${PROJECT_BINARY_DIR}/qt/qml;${QML_IMPORT_PATH}" CACHE STRING "For LSP" FORCE)

if(WITH_SUBMODULE_WAYLIB)
    set(WITH_SUBMODULE_QWLROOTS ON)
    add_subdirectory(waylib)
    set(QML_IMPORT_PATH "${PROJECT_BINARY_DIR}/src/server;${QML_IMPORT_PATH}" CACHE STRING "For LSP" FORCE)
else()
    find_package(Waylib REQUIRED Server)
endif()

# PKG-CONFIG
find_package(PkgConfig)

find_package(Qt6 CONFIG REQUIRED Core DBus Gui Qml Quick QuickControls2 LinguistTools Test QuickTest)
qt_standard_project_setup(REQUIRES 6.6)

# Set constants
set(TREELAND_DATA_DIR           "${CMAKE_INSTALL_FULL_DATADIR}/treeland/"               CACHE PATH      "treeland data install directory")
set(COMPONENTS_TRANSLATION_DIR  "${TREELAND_DATA_DIR}/translations"                  CACHE PATH      "Components translations directory")

add_compile_definitions("TREELAND_DATA_DIR=\"${TREELAND_DATA_DIR}\"")

add_subdirectory(src)
add_subdirectory(data)

if (BUILD_TEST_CLIENTS)
    add_subdirectory(tests)
endif()

# Display feature summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
