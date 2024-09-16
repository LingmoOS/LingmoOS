cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(deepin-system-monitor-plugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)

set(APP_RES_ROOT asserts)
set(APP_RESOURCES ${APP_RES_ROOT}/${PROJECT_NAME}.qrc)

# Sources files
file(GLOB SRCS "*.h" "*.cpp" "gui/*.h" "gui/*.cpp" "dbus/*.h" "dbus/*.cpp")

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)

pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(DdeDockInterface REQUIRED dde-dock)

if (DdeDockInterface_FOUND)
    message("dde-dock founded: ${DdeDockInterface_VERSION}")
    string(REGEX MATCH "^[0-9]" VERSION_MAJOR ${DdeDockInterface_VERSION})
    if (VERSION_MAJOR STREQUAL "6" OR VERSION_MAJOR STRGREATER "6")
        add_definitions(-DDDE_DOCK_MAJORV6)
    endif()
endif()

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PROJECT_NAME} SHARED ${SRCS} ${APP_RESOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)

include_directories(${DtkCore_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${DdeDockInterface_INCLUDE_DIRS}
  ${DtkWidget_INCLUDE_DIRS}
  ${QGSettings_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${Qt5DBus_LIBRARIES}
  ${DtkWidget_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${QGSettings_LIBRARIES}
)

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-Map=deepin-system-monitor-plugin.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-Map=deepin-system-monitor-plugin.map")

#生成qm
file(GLOB APP_TS_FILES LIST_DIRECTORIES false translations/*.ts)
qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})
add_custom_target(PLUGIN_QMFILES ALL DEPENDS ${APP_QM_FILES})

INSTALL(FILES com.deepin.dde.dock.module.system-monitor.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)
#INSTALL(FILES com.deepin.system.monitor.plugin.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)
INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
INSTALL(FILES asserts/icons/deepin/builtin/actions/deepin-system-monitor_20px.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/share/dde-dock/icons/dcc-setting
  RENAME deepin-system-monitor.svg
)
#安装翻译文件
install(FILES ${APP_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-system-monitor-plugin/translations)
