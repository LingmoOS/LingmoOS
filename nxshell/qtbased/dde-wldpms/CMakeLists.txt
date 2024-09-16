cmake_minimum_required(VERSION 3.7)

project(dde_wldpms)

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -z relro -z now -z noexecstack -pie")

set(TARGET_NAME dde_wldpms)

add_executable(${TARGET_NAME} src/main.cpp)

find_package(DWayland REQUIRED)
# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus Gui REQUIRED)

target_link_libraries(${TARGET_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::DBus
  DWaylandClient
)
