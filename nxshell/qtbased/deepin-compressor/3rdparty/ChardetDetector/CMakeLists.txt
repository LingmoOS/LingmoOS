#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(LIB_NAME compressor-ChardetDetector)
project(ChardetDetector)

set(CMAKE_AUTOMOC OFF) #关闭moc
set(CMAKE_AUTORCC OFF)

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files RELATIVE ${PROJECT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE h_files RELATIVE ${PROJECT_SOURCE_DIR} *.h)
file(GLOB_RECURSE tab_files RELATIVE ${PROJECT_SOURCE_DIR} *.tab)

#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})

#导出头文件
set(output_h_files
    ${PROJECT_SOURCE_DIR}/chardet.h
    ${PROJECT_SOURCE_DIR}/version.h
)
file(COPY ${output_h_files} DESTINATION ${HFILES_OUTPUT_PATH})

#生成动静态库
add_library(${LIB_NAME} STATIC ${c_files} ${h_files} ${tab_files})
