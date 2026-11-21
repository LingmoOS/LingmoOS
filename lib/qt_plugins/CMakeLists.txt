cmake_minimum_required(VERSION 3.16)

# C is needed for the generated Wayland protocol marshalling.
project(lingmo-qt-plugins LANGUAGES C CXX)

include(CTest)

add_subdirectory(scrolling)
add_subdirectory(platformtheme)
add_subdirectory(imageformat)
