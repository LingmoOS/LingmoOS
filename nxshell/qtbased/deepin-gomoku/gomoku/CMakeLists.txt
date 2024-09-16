cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
     message(WARNING "Not defined version ,about dialog version set 1.2.2")
    set(VERSION 1.2.2)
endif ()

#common resource names
set(APP_RES_DIR "assets")
set(APP_BIN_NAME "deepin-gomoku")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-gomoku.desktop")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")

project(${APP_BIN_NAME})

#添加ci编译参数
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-Wl,--as-needed -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

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

#angr安全加固
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z relro -z now -z noexecstack -pie")

#配置版本号
configure_file(${APP_RES_DIR}/environments.h.in environments.h @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all app own subdirectorys
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()

file(GLOB_RECURSE Gomoku_SRC ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5 COMPONENTS Multimedia REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

#添加源文件以及资源文件
ADD_EXECUTABLE(${PROJECT_NAME} ${Gomoku_SRC} ${APP_QRC})
#添加头文件
target_include_directories(${PROJECT_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})
#添加链接库
target_link_libraries(${PROJECT_NAME}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    Qt5::Multimedia
)

#根据环境修改/usr 目录
if(DEFINED ENV{PREFIX})
   set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
else()
   set(CMAKE_INSTALL_PREFIX /usr)
endif()

# Install files
#install(TARGETS deepin-gomoku DESTINATION bin)

#install(DIRECTORY ${APP_RES_DIR}/deepin-gomoku
#        DESTINATION /usr/share/deepin-manual/manual-assets/application/)
#install(FILES ${APP_DESKTOP} DESTINATION share/applications)

install(FILES ${APP_DESKTOP} DESTINATION share/applications)
install(FILES assets/deepin-gomoku.desktop DESTINATION share/applications)
install(FILES assets/resources/desktop/com.deepin.gomoku.svg
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets/deepin-gomoku DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/)
