cmake_minimum_required(VERSION 3.14)

project(lingmo-screenshot LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core DBus Gui Widgets Quick LinguistTools REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

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
  Qt6::Core
  Qt6::DBus
  Qt6::Gui
  Qt6::Widgets
  Qt6::Quick
  Qt6::Core5Compat
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-screenshot/translations)

install(TARGETS lingmo-screenshot RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    lingmo-screenshot.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
