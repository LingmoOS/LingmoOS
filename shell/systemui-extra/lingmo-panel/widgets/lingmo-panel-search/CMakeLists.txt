cmake_minimum_required(VERSION 3.16)

project(lingmo-panel-search VERSION 4.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick DBus LinguistTools REQUIRED)

set(PROJECT_SOURCES
        plugin/search-button.h
        plugin/search-plugin.h
        plugin/search-plugin.cpp
        plugin/search-button.cpp
)
file(GLOB TS_FILES translations/*.ts)
set(SEARCH_WIDGET_DIR "/usr/share/lingmo/widgets/org.lingmo.panel.search")
set(TRANSLATION_FILE_DIR "${SEARCH_WIDGET_DIR}/translations")
add_compile_definitions(TRANSLATION_FILE_DIR="${TRANSLATION_FILE_DIR}"
        LINGMO_TASK_MANAGER_VERSION="${LINGMO_TASK_MANAGER_VERSION}")

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}  ${TS_FILES})
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    add_library(${PROJECT_NAME} SHARED MANUAL_FINALIZATION ${PROJECT_SOURCES} ${QM_FILES})
else()
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})
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

install(DIRECTORY "widget/" DESTINATION ${SEARCH_WIDGET_DIR})
install(FILES ${QM_FILES} DESTINATION "${TRANSLATION_FILE_DIR}")
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/panel/search")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/panel/search")
