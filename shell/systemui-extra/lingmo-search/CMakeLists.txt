cmake_minimum_required(VERSION 3.14)
project(lingmo-search LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

add_subdirectory(libchinese-segmentation)
add_subdirectory(libsearch)
add_subdirectory(frontend)
add_subdirectory(lingmosearch-systemdbus)
add_subdirectory(search-ukcc-plugin)
add_subdirectory(lingmo-search-service)
add_subdirectory(lingmo-search-app-data-service)
add_subdirectory(lingmo-search-service-dir-manager)

if (BUILD_TEST)
    add_subdirectory(tests)
endif ()