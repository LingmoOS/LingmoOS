cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    message(WARNING "Not defined version, about dialog version set 5.10.0")
    set(VERSION 5.10.0)
endif ()

#common resource names
set(APP_RES_DIR "assets")
set(APP_BIN_NAME "dde-calendar")
set(APP_DESKTOP "${APP_RES_DIR}/dde-calendar.desktop")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")
set(APP_SERVICE_IN "${APP_RES_DIR}/dbus/com.deepin.Calendar.service.in")

project(${APP_BIN_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")


if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-needed")
endif ()

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

configure_file(${APP_RES_DIR}/environments.h.in environments.h @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all app own subdirectories
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()

file(GLOB_RECURSE Calendar_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(Qt5Network REQUIRED)

set_source_files_properties(src/dbus/org.deepin.dde.ControlCenter.xml PROPERTIES CLASSNAME ControlCenterProxy)
qt5_add_dbus_interface(Calendar_SRC src/dbus/org.deepin.dde.ControlCenter.xml controlCenterProxy)

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${Calendar_SRC} ${APP_QRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR} ../calendar-common/src)
target_link_libraries(${PROJECT_NAME}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    commondata
)

#修改dbus服务文件
set (EXEC_DDE_CALENDAR "${CMAKE_INSTALL_FULL_BINDIR}/${APP_BIN_NAME}" CACHE STRING "Command to execute dde-calendar")

configure_file(${APP_SERVICE_IN} ${CMAKE_CURRENT_BINARY_DIR}/dbus/com.deepin.Calendar.service @ONLY)

# Install files
install(TARGETS dde-calendar DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${APP_RES_DIR}/dde-calendar
        DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)
install(FILES ${APP_DESKTOP} DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbus/com.deepin.Calendar.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
install(FILES assets/org.deepin.calendar.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)