cmake_minimum_required(VERSION 3.14)
project(lingmo-preic LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp
    welcomepage.cpp
    installpage.cpp
    translator.cpp
    ripplebutton.cpp
)

set(HEADERS
    mainwindow.h
    welcomepage.h
    installpage.h
    translator.h
    ripplebutton.h
)

set(RESOURCES
    resources.qrc
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

qt_add_resources(QRC_SOURCES ${RESOURCES})

add_executable(lingmo-preic
    ${SOURCES}
    ${HEADERS}
    ${QRC_SOURCES}
    ${QM_FILES}
)

target_link_libraries(lingmo-preic PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

install(TARGETS lingmo-preic RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
