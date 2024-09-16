cmake_minimum_required(VERSION 3.7)
project(dde-calendar-service-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(APP_BIN_NAME "dde-calendar-service-test")
set(APP_SERVICE_DIR "${CMAKE_SOURCE_DIR}/calendar-service")
set(APP_SERVICE_RES_DIR "${APP_SERVICE_DIR}/assets")
set(APP_QRC "${APP_SERVICE_RES_DIR}/resources.qrc")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
    Sql
REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(GTest REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Sql
    ${DtkCore_LIBRARIES}
)

include_directories(${APP_SERVICE_DIR}/src)

SUBDIRLIST(all_src ${APP_SERVICE_DIR}/src)

#Include all app own subdirectories
foreach(subdir ${all_src})
    include_directories(${APP_SERVICE_DIR}/src/${subdir})
endforeach()

file(GLOB_RECURSE CALENDARSERVICE_SRCS ${APP_SERVICE_DIR}/src/*.cpp)
include_directories(../../calendar-basicstruct/)
list(REMOVE_ITEM CALENDARSERVICE_SRCS ${APP_SERVICE_DIR}/src/main.cpp)

#test src
SUBDIRLIST(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(subdir ${TEST_SRC})
    include_directories(${subdir})
endforeach()

set(HL_DATABASE_DIR ${APP_SERVICE_RES_DIR}/data/huangli.db)
set(SD_DATABASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.db)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in config.h @ONLY)

file(GLOB_RECURSE Calendar_Service_TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

list(APPEND CMAKE_MODULE_PATH "${ASAN_CMAKE_DIR}")
include(asan)

add_executable(${APP_BIN_NAME} ${Calendar_Service_TEST_SRC} ${CALENDARSERVICE_SRCS} ${APP_QRC})

target_link_libraries(${APP_BIN_NAME} 
    ${LINK_LIBS}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    basestruct
)