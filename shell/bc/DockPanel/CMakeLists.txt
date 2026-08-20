cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME lingmo-dock)
project(${PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 CONFIG REQUIRED Widgets DBus Concurrent LinguistTools QuickControls2)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

find_package(KF6WindowSystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB_LIBS REQUIRED xcb)

find_path(QT6_GUI_PRIVATE_INCLUDE_DIR qpa/qplatformnativeinterface.h
    HINTS ${Qt6Gui_INCLUDE_DIRS} PATH_SUFFIXES QtGui)
if(QT6_GUI_PRIVATE_INCLUDE_DIR)
    include_directories(${QT6_GUI_PRIVATE_INCLUDE_DIR})
endif()

set(SRCS
    src/applicationitem.h
    src/applicationmodel.cpp
    src/docksettings.cpp
    src/iconthemeimageprovider.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/systemappmonitor.cpp
    src/systemappitem.cpp
    src/processprovider.cpp
    src/trashmanager.cpp
    src/utils.cpp
    src/xwindowinterface.cpp
    src/activity.cpp

    src/fakewindow.cpp
)

set(RESOURCES
    resources.qrc
)

qt_add_dbus_adaptor(DBUS_SOURCES
                     src/com.lingmo.Dock.xml
                     src/mainwindow.h MainWindow)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME} ${SRCS} ${DBUS_SOURCES} ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${XCB_LIBS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Concurrent
        Qt6::DBus
        Qt6::Core5Compat
        KF6::WindowSystem
        ${XCB_LIBS_LIBRARIES}
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
install(FILES lingmo-dock-list.conf DESTINATION /etc)
