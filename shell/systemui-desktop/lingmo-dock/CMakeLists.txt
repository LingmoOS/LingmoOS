cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME lingmo-dock)
project(${PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 CONFIG REQUIRED Widgets DBus X11Extras Concurrent LinguistTools QuickControls2)

find_package(KF5WindowSystem REQUIRED)

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

qt5_add_dbus_adaptor(DBUS_SOURCES
                     src/com.lingmo.Dock.xml
                     src/mainwindow.h MainWindow)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME} ${SRCS} ${DBUS_SOURCES} ${RESOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Quick
        Qt5::QuickControls2
        Qt5::X11Extras
        Qt5::Concurrent
        Qt5::DBus
        KF5::WindowSystem
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
install(FILES lingmo-dock-list.conf DESTINATION /etc)
