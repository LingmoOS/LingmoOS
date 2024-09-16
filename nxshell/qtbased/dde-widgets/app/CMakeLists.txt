# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

PROJECT(frame)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(REQUIRED_QT_VERSION 5.11.3)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets DBus Concurrent)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkWidget REQUIRED)

find_package(PkgConfig REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

set(XML2CPP_DIR ${PROJECT_SOURCE_DIR}/utils/dbus/xml2cpp)

#dbus xml
find_package(DtkTools REQUIRED)

function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND ${DTK_XML2CPP} -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${XML2CPP_DIR})
endfunction(generation_dbus_interface)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Appearance1.xml
    Appearance
    ${XML2CPP_DIR}/appearance_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.daemon.Dock1.xml
    Dock
    ${XML2CPP_DIR}/dock_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Display1.Monitor.xml
    Monitor
    ${XML2CPP_DIR}/monitor_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Display1.xml
    Display
    ${XML2CPP_DIR}/display_interface
)

file(GLOB DBUS_TYPES "utils/dbus/xml2cpp/types/*.*")
file(GLOB DBUS_INTERFACES "utils/dbus/xml2cpp/*.*")

include(src.cmake)


ADD_EXECUTABLE(${PROJECT_NAME}
    main.cpp
    ${HEADERS}
    ${SOURCES}
    ${DBUS_XML}
    ${DBUS_TYPES}
    ${DBUS_INTERFACES}
)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME dde-widgets)

set(COMMON_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
    ${DtkWidget_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${COMMON_LIBS})
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../interface)
target_link_libraries(${PROJECT_NAME} PRIVATE dde-widgets)

target_include_directories(${PROJECT_NAME} PUBLIC
    Qt5::Concurrent
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${DtkWidget_INCLUDE_DIRS}
    dbus/xml2cpp/
)

install(FILES services/org.deepin.dde.Widgets1.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

configure_file(
	${CMAKE_SOURCE_DIR}/app/services/systemd/dde-widgets.service.in
	${CMAKE_CURRENT_BINARY_DIR}/dde-widgets.service
	@ONLY)
set(SYSTEMD_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dde-widgets.service
)

macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/user/${wantsdir}/)
endmacro(install_symlink)

install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
install(DIRECTORY DESTINATION lib/systemd/user/dde-osd.target.wants/)
install_symlink(dde-widgets.service dde-osd.target.wants)
