cmake_minimum_required(VERSION 3.14)

project(lingmo-screenshot LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core DBus Gui Widgets Quick LinguistTools REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/screenshotview.cpp
    qml.qrc
)

add_executable(lingmo-screenshot
  ${PROJECT_SOURCES}
)

target_link_libraries(lingmo-screenshot
  PRIVATE
  Qt5::Core
  Qt5::DBus
  Qt5::Gui
  Qt5::Widgets
  Qt5::Quick
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-screenshot/translations)

install(TARGETS lingmo-screenshot RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    lingmo-screenshot.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
