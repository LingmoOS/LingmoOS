cmake_minimum_required(VERSION 3.14)

project(lingmo-texteditor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT Core Gui Quick QuickControls2 LinguistTools)
find_package(Qt5 REQUIRED ${QT})

find_package(Qt5 COMPONENTS Core Widgets Quick REQUIRED)
find_package(KF5SyntaxHighlighting)

set(PROJECT_SOURCES
        src/main.cpp
        src/documenthandler.cpp
        src/highlightmodel.cpp
        src/texteditor.cpp
        qml.qrc
)

add_executable(lingmo-texteditor
  ${PROJECT_SOURCES}
)

target_link_libraries(lingmo-texteditor
  PRIVATE
  Qt5::Core
  Qt5::Widgets
  Qt5::Quick
  KF5::SyntaxHighlighting
  )

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
  
install(TARGETS lingmo-texteditor RUNTIME DESTINATION /usr/bin)
install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-texteditor/translations)

install(FILES
    lingmo-texteditor.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
