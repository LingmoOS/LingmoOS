project(minimal)

find_package(ECM REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(OCEANICONS_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ocean-icons/)

find_package(Qt6 REQUIRED Core Quick Multimedia Test Widgets QuickControls2)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(3rdparty)
add_subdirectory(src)

