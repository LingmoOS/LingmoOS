cmake_minimum_required(VERSION 3.10)

add_subdirectory(ocean-dock-plugins)

if (DFM_BUILD_WITH_QT6)
    add_subdirectory(ocean-shell-plugins)
endif()
