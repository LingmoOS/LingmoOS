cmake_minimum_required(VERSION 3.16)

project(LingmoFramework
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(CTest)
include(LingmoFrameworkHelpers)

add_subdirectory(clients)
add_subdirectory(files)

lingmo_add_translations(lingmo-framework SOURCES clients common files)
