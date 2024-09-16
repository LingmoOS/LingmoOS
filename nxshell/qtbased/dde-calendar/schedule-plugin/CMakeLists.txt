cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

#common resource names
set(APP_RES_DIR "assets")
set(APP_BIN_NAME "uosschedulex-plugin")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")

project(${APP_BIN_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()
add_definitions("-DSCHEDULEPLUGIN_LIBRARY")
add_definitions("-DQT_DEPRECATED_WARNINGS")
#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
endif()

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all app own subdirectories
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()

file(GLOB_RECURSE Schedule_Plugin_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)


include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Tell CMake to create the lib
add_library(${PROJECT_NAME} SHARED ${Schedule_Plugin_SRC} ${APP_QRC} src/interface/service.h)

target_include_directories(${PROJECT_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR} ../calendar-common/src)

target_link_libraries(${PROJECT_NAME}
    ${Qt5Svg_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    commondata
)

# Install files
install(TARGETS ${PROJECT_NAME} DESTINATION lib/deepin-aiassistant/serivce-plugins)
