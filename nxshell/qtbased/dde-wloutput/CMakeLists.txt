cmake_minimum_required(VERSION 3.7)

project(dde_wloutput)

set(TARGET_NAME dde_wloutput)

add_executable(${TARGET_NAME} src/main.cpp)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(DWayland) # DWayland的LIB和INCLUDE的cmake配置暂时有问题，目前通过全路径寻找

target_link_libraries(${TARGET_NAME}
    Qt5::Core
    DWaylandClient
)