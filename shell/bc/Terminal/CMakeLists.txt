cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME lingmo-terminal)
project(${PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT Core Gui Quick QuickControls2 Widgets DBus LinguistTools)
find_package(Qt6 REQUIRED ${QT})
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

# Get the installation directory from qmake
get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif()

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML
    OUTPUT_VARIABLE INSTALL_QMLDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(INSTALL_QMLDIR)
    message(STATUS "qml directory:" "${INSTALL_QMLDIR}")
else()
    message(FATAL_ERROR "qml directory cannot be detected.")
endif()

set(SRCS
    src/main.cpp
    src/processhelper.h
    src/processhelper.cpp
    src/utils.cpp
    src/fonts.cpp
)

set(RESOURCES
    src/resources.qrc
)

add_subdirectory(qmltermwidget)
add_executable(${PROJECT_NAME} ${SRCS} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        Qt6::DBus
        Qt6::Core5Compat
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-terminal/translations)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin)

install(FILES
    lingmo-terminal.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
