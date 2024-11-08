cmake_minimum_required(VERSION 3.16)

project(lingmo-panel-taskView VERSION 4.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick DBus LinguistTools REQUIRED)

set(PROJECT_SOURCES
        plugin/task-view-button.h
        plugin/task-view-plugin.h
        plugin/task-view-plugin.cpp
        plugin/task-view-button.cpp
)

add_compile_definitions(LINGMO_TASK_MANAGER_VERSION="${LINGMO_TASK_MANAGER_VERSION}")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    add_library(${PROJECT_NAME} SHARED MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
endif()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::DBus
)

install(DIRECTORY "widget/" DESTINATION "/usr/share/lingmo/widgets/org.lingmo.panel.taskView")
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/panel/taskView")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/panel/taskView")
