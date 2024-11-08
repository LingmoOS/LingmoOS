cmake_minimum_required(VERSION 3.14)

project(lingmo-calendar)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_CALENDAR_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Quick LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Quick LinguistTools REQUIRED)

set(PROJECT_SOURCES
        plugin/lingmo-calendar-plugin.h plugin/lingmo-calendar-plugin.cpp
        plugin/lunar-calendar-info.h plugin/lunar-calendar-info.cpp
        plugin/calendar-model.h plugin/calendar-model.cpp
        plugin/calendar-item.h plugin/calendar-item.cpp
)

set(LINGMO_CALENDAR_DATA_DIR "/usr/share/lingmo/widgets/org.lingmo.calendar")
set(LINGMO_CALENDAR_TRANSLATION_DIR "${LINGMO_CALENDAR_DATA_DIR}/translations")

qt5_add_resources(QRC_FILES plugin/res.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} translations/lingmo-calendar_zh_CN.ts OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED MANUAL_FINALIZATION ${PROJECT_SOURCES} ${QRC_FILES} ${QM_FILES})

else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} translations/lingmo-calendar_zh_CN.ts OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QRC_FILES} ${QM_FILES})
endif()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)

install(DIRECTORY "widget/" DESTINATION ${LINGMO_CALENDAR_DATA_DIR})
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/calendar")
install(FILES ${QM_FILES} DESTINATION ${LINGMO_CALENDAR_TRANSLATION_DIR})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/calendar")
