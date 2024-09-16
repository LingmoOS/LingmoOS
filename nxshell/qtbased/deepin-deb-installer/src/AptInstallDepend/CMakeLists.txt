# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED VERSION)
    set(VERSION 5.3.9)
endif ()

project(AptInstallDepend)
set (EXE_NAME deepin-deb-installer-dependsInstall)
set(LINK_LIBS
    Qt5::Core
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

#add process directories
string(REGEX REPLACE "(.*)/(.*)" "\\1" PROJECT_INIT_PATH  ${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_INIT_PATH}/deb-installer/process)

file(GLOB_RECURSE AUTH_CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${PROJECT_INIT_PATH}/deb-installer/process/*.cpp)
add_executable (${EXE_NAME}
    ${AUTH_CPP_FILES}
)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
set(CMAKE_INSTALL_PREFIX /usr)
# Install files
install(TARGETS ${EXE_NAME} DESTINATION bin)
target_link_libraries(${EXE_NAME} ${LINK_LIBS} )
target_link_libraries(${EXE_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui)
