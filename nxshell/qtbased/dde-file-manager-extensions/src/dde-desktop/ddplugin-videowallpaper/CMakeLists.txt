cmake_minimum_required(VERSION 3.10)

project(ddplugin-videowallpaper)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(PkgConfig REQUIRED)
find_package(dfm-base REQUIRED)
find_package(dfm-framework REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(Dtk COMPONENTS Core REQUIRED)
pkg_search_module(libdmr REQUIRED libdmr)

if (libdmr_FOUND)
    add_definitions(-DUSE_LIBDMR)
    message("found libdmr...")
    set(Media_INCLUDE_DIRS
            ${libdmr_INCLUDE_DIRS}
        )
    set(Media_LIBRARIES
            ${libdmr_LIBRARIES}
        )
else()
    message(FATAL_ERROR "libdmr not found")
    find_package(Qt5 COMPONENTS Multimedia MultimediaWidgets REQUIRED)
    set(Media_INCLUDE_DIRS
        ${Qt5MultimediaWidgets_INCLUDE_DIRS}
        ${Qt5Multimedia_INCLUDE_DIRS}
        )
    set(Media_LIBRARIES
        ${Qt5MultimediaWidgets_LIBRARIES}
        ${Qt5Multimedia_LIBRARIES}
        )
endif()

# 查找匹配 ddplugin-videowallpaper*.ts 的文件列表
file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/ddplugin-videowallpaper*.ts")

# 添加 lrelease 命令，传递 TS_FILES 列表
foreach(TS_FILE ${TS_FILES})
    execute_process(
       COMMAND lrelease ${TS_FILE}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

qt5_add_resources(QRC_RESOURCES ts.qrc)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
    ${QRC_RESOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DtkCore_INCLUDE_DIRS}
    ${dfm-framework_INCLUDE_DIRS}
    ${dfm-base_INCLUDE_DIRS}
    ${Media_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Concurrent
    ${DtkCore_LIBRARIES}
    ${dfm-framework_LIBRARIES}
    ${dfm-base_LIBRARIES}
    ${Media_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_EDGE_DIR}
)

dconfig_meta_files(APPID "org.deepin.dde.file-manager"
    BASE "${CMAKE_SOURCE_DIR}/assets/configs"
    FILES "${CMAKE_SOURCE_DIR}/assets/configs/org.deepin.dde.file-manager.desktop.videowallpaper.json"
)

# copy install file for packgage
SET(DEBIAN_PATH ${CMAKE_SOURCE_DIR}/debian)
FILE(COPY debian/dde-desktop-videowallpaper-plugin.install DESTINATION ${DEBIAN_PATH})
message("copy video wallpaper install file to ${DEBIAN_PATH}")
