# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(REQUIRED_QT_VERSION 5.11.3)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkWidget REQUIRED)

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Test)

# generate moc_predefs.h
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

set(DTK_HAS_UNIT_TEST ON)
# for coverage
if (DTK_HAS_UNIT_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage")
endif()

include(../app/src.cmake)

list(APPEND HEADERS
    ../interface/widgetsglobal.h
    ../interface/widgetsinterface.h
    ../interface/widgetsinterface_p.h)

list(APPEND SOURCES
    ../interface/widgetsinterface.cpp)

list(
    APPEND SOURCES
    ut_widgetsmanager.cpp
    ut_instancemodel.cpp
)

file(GLOB DBUS_TYPES "../app/utils/dbus/xml2cpp/types/*.*")
file(GLOB DBUS_INTERFACES "../app/utils/dbus/xml2cpp/*.*")

ADD_EXECUTABLE(dde-widgets-test
    main.cpp
    ${HEADERS}
    ${SOURCES}
    ${DBUS_XML}
    ${DBUS_TYPES}
    ${DBUS_INTERFACES}
    ${DBUS_EXTENDED}
)

target_include_directories(dde-widgets-test PUBLIC
    ../app
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

set(COMMON_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

target_link_libraries(dde-widgets-test PUBLIC ${COMMON_LIBS}
    -lgtest
    -lpthread
)
