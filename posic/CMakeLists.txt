cmake_minimum_required(VERSION 3.5)
project(lingmo-welcome LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

set(SOURCES
    main.cpp
    welcome_window.cpp
    dock_page.cpp
    theme_page.cpp
    finish_page.cpp
    welcome_manager.cpp
    ripple_button.cpp
)

set(HEADERS
    welcome_window.h
    dock_page.h
    theme_page.h
    finish_page.h
    welcome_manager.h
    ripple_button.h
)

set(RESOURCES
    welcome.qrc
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
    execute_process(COMMAND lrelease ${_ts_file})
endforeach()

# 生成 qrc 文件
qt5_add_resources(QRC_SOURCES ${RESOURCES})

add_executable(lingmo-welcome
    ${SOURCES}
    ${HEADERS}
    ${QRC_SOURCES}
    ${QM_FILES}
)

target_link_libraries(lingmo-welcome PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
)

install(TARGETS lingmo-welcome RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)