cmake_minimum_required(VERSION 3.10)

add_definitions(-DDFM_ORGANIZATION_NAME="lingmo")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(CMAKE_VERBOSE_MAKEFILEON ON)
#    message("debug type open sanitize check")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_L_FLAGS "${CMAKE_L_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

add_subdirectory(ocean-file-manager)
add_subdirectory(ocean-file-manager-daemon)
add_subdirectory(ocean-file-manager-preview)
add_subdirectory(ocean-file-dialog)
add_subdirectory(ocean-file-dialog-x11)
add_subdirectory(ocean-file-dialog-wayland)
if(NOT DFM_BUILD_WITH_QT6)
    add_subdirectory(ocean-desktop) # as a plugin of ocean-shell in Qt6
endif()
