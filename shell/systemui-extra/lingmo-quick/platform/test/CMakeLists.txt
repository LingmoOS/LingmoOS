cmake_minimum_required(VERSION 3.14)
project(platform-test)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml Widgets REQUIRED)

set(QRC_FILES qml.qrc)
set(PROJECT_SOURCES
        main.cpp
        ${QRC_FILES}
)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Qml
)
