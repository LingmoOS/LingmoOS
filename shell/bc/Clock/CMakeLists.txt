cmake_minimum_required(VERSION 3.5)

project(lingmo-clock LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick QuickControls2 LinguistTools REQUIRED)
find_package(KF5Notifications)

set(PROJECT_SOURCES
    src/main.cpp
    src/resources.qrc
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

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${QM_FILES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    KF5::Notifications
)

install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin)
install(FILES org.lingmo.clock.desktop DESTINATION /usr/share/applications)