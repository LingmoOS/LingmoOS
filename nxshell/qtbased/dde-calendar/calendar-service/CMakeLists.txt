cmake_minimum_required(VERSION 3.7)
project(dde-calendar-service)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

set(APP_RES_DIR "assets")
set( SERVER_SERVICE_IN "${APP_RES_DIR}/data/com.deepin.dataserver.Calendar.service.in")
set( APP_SERVICE_IN "${APP_RES_DIR}/data/dde-calendar.service.in")
set( SERVER_SYSTEMD_SERVICE "${APP_RES_DIR}/data/com.dde.calendarserver.calendar.service")
set(APP_SYSTEMD_TIMER "${APP_RES_DIR}/data/com.dde.calendarserver.calendar.timer")
set(AUTOSTART_DESKTOP "${APP_RES_DIR}/dde-calendar-service.desktop")
set(HUANGLIDB "${APP_RES_DIR}/data/huangli.db")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")


# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
    Sql
REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Sql
    ${DtkCore_LIBRARIES}
    commondata
    -lpthread
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all app own subdirectories
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()
file(GLOB_RECURSE CALENDARSERVICE_SRCS ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
#添加资源文件
QT5_ADD_RESOURCES(CALENDARSERVICE_SRCS ${APP_QRC})


#后端程序自动退出宏控制
if (NOT (CMAKE_BUILD_TYPE MATCHES Debug))
  add_definitions(-DCALENDAR_SERVICE_AUTO_EXIT)
endif()

add_executable(${PROJECT_NAME} ${CALENDARSERVICE_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR} ../calendar-common/src)

target_link_libraries(${PROJECT_NAME}
    ${LINK_LIBS}
    )

set (SERVICE_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/deepin-daemon" CACHE STRING "Install dir for dde-calendar-service")

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    CALENDAR_SERVICE_PATH="${SERVICE_INSTALL_DIR}"
)


#修改dbus服务文件
set (EXEC_DDE_CALENDAR_SERVICE "${SERVICE_INSTALL_DIR}/dde-calendar-service" CACHE STRING "Command to execute dde-calendar-service")

configure_file(${SERVER_SERVICE_IN} ${CMAKE_CURRENT_BINARY_DIR}/dbus/com.deepin.dataserver.Calendar.service @ONLY)
configure_file(${APP_SERVICE_IN} ${CMAKE_CURRENT_BINARY_DIR}/assets/dde-calendar.service @ONLY)

# Install files
install(TARGETS ${PROJECT_NAME} DESTINATION ${SERVICE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbus/com.deepin.dataserver.Calendar.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services/)
install(FILES ${AUTOSTART_DESKTOP} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart/)
install(FILES ${HUANGLIDB} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-calendar/data/)
install(FILES ${SERVER_SYSTEMD_SERVICE} DESTINATION lib/systemd/user/)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/assets/dde-calendar.service DESTINATION lib/systemd/user/)
install(FILES ${APP_SYSTEMD_TIMER} DESTINATION lib/systemd/user/)
