project(framework-test VERSION 4.1 LANGUAGES CXX)

include_directories(../widget-ui)
include_directories(../widget)
include_directories(../config)
include_directories(../view)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

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
        lingmo-quick::framework
)

add_subdirectory(org.lingmo.testWidget)
