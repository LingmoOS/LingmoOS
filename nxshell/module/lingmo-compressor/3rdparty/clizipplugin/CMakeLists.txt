project(clizipplugin)

cmake_minimum_required(VERSION 3.9.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(LIB_NAME clizipplugin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5 REQUIRED Widgets)
find_package(Qt5Concurrent)
find_package(Qt5Xml)
find_package(Qt5Svg REQUIRED SvgWidgets)
find_package(KF5Codecs REQUIRED)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(DtkCore REQUIRED dtkcore)
include_directories(${DtkCore_INCLUDE_DIRS})

set(LIBS_INCLUDE_DIRS 
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
)


include_directories(${PROJECT_SOURCE_DIR}/../ChardetDetector)
include_directories(${PROJECT_SOURCE_DIR}/../interface)

file(GLOB_RECURSE json_files  *.json)



file(GLOB_RECURSE c_files  *.cpp)
file(GLOB_RECURSE h_files  *.h)
file(GLOB_RECURSE json_files  *.json)
include_directories(${PROJECT_SOURCE_DIR})

add_library(${LIB_NAME} SHARED  ${c_files} ${h_files} ${json_files})
target_link_libraries(${LIB_NAME} Qt5::Widgets)
target_link_libraries(${LIB_NAME} ${LIBS_INCLUDE_DIRS})
target_link_libraries( ${LIB_NAME} compressor-interface)
