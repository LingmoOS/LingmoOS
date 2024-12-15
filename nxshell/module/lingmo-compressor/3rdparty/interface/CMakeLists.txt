#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(LIB_NAME compressor-interface)
project(interface)

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5Codecs REQUIRED)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
pkg_search_module(MOUNT REQUIRED mount)

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files RELATIVE ${PROJECT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE h_files RELATIVE ${PROJECT_SOURCE_DIR} *.h)

#指定头文件路径
include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${MOUNT_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${KF5Codecs_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/archiveinterface)
include_directories(${PROJECT_SOURCE_DIR}/plugin)
include_directories(${PROJECT_SOURCE_DIR}/process)
include_directories(${HFILES_OUTPUT_PATH})


#导出头文件
set(output_h_files
    ${PROJECT_SOURCE_DIR}/common.h
    ${PROJECT_SOURCE_DIR}/commonstruct.h
    ${PROJECT_SOURCE_DIR}/datamanager.h
    ${PROJECT_SOURCE_DIR}/queries.h
    ${PROJECT_SOURCE_DIR}/archiveinterface/archiveinterface.h
    ${PROJECT_SOURCE_DIR}/archiveinterface/cliinterface.h
    ${PROJECT_SOURCE_DIR}/archiveinterface/cliproperties.h
    ${PROJECT_SOURCE_DIR}/plugin/kpluginfactory.h
    ${PROJECT_SOURCE_DIR}/plugin/kpluginloader.h
    ${PROJECT_SOURCE_DIR}/plugin/kpluginmetadata.h
    ${PROJECT_SOURCE_DIR}/process/kprocess.h
    ${PROJECT_SOURCE_DIR}/process/kpty.h
    ${PROJECT_SOURCE_DIR}/process/kpty_p.h
    ${PROJECT_SOURCE_DIR}/process/kptydevice.h
    ${PROJECT_SOURCE_DIR}/process/kptyprocess.h
)
file(COPY ${output_h_files} DESTINATION ${HFILES_OUTPUT_PATH})

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#生成动静态库
add_library(${LIB_NAME} SHARED ${h_files} ${c_files})

#链接动静态库
target_link_libraries(${LIB_NAME}
    Qt5::Widgets
    KF5::Codecs
    ${DtkWidget_LIBRARIES}
    ${MOUNT_LIBRARIES}
    compressor-ChardetDetector
)

