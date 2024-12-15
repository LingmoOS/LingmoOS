#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#定义cmake版本
set(LIB_NAME gzplugin)
project(${LIB_NAME})

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5Codecs REQUIRED)
#find_package(KF5Archive)
include(FindPkgConfig)

#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files RELATIVE ${PROJECT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE h_files RELATIVE ${PROJECT_SOURCE_DIR} *.h)
file(GLOB_RECURSE json_files RELATIVE ${PROJECT_SOURCE_DIR} *.json)

#生成动静态库
add_library(${LIB_NAME} SHARED ${c_files} ${h_files} ${json_files})

#链接动静态库
target_link_libraries(${LIB_NAME}
    Qt5::Widgets
    KF5::Codecs
#    KF5::Archive
    compressor-singlefile
)
