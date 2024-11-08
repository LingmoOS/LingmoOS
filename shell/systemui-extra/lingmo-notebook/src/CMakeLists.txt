cmake_minimum_required(VERSION 3.5)

project(lingmo-notebook LANGUAGES CXX C)

include(${PROJECT_SOURCE_DIR}/../cmake/common.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/common)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

option(TEST_DEBUG "option for debug" OFF)

if (TEST_DEBUG) 
	add_definitions(-DTEST_DEBUG)
    add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
    add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")
endif(TEST_DEBUG)

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus Sql Concurrent REQUIRED)

if (TEST_DEBUG)
    execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/../sh/generate_ui.sh
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif(TEST_DEBUG)
execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/../sh/update_translations.sh
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/../sh/apply_translations.sh
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/utils
                    ${PROJECT_SOURCE_DIR}/custom_ui
                    ${PROJECT_SOURCE_DIR}/font_button
                    ${common_SOURCE_DIR}
                    )

file(GLOB SRC 
    "*.cpp"
    "*.h"
    "utils/*.cpp"
    "utils/*.h"
    "custom_ui/*.cpp"
    "custom_ui/*.h"
    "font_button/*.cpp"
    "font_button/*.h"
)

SET(ui_RESOURCE ${PROJECT_SOURCE_DIR}/src.qrc
                ${PROJECT_SOURCE_DIR}/image.qrc
                ${PROJECT_SOURCE_DIR}/pic/pic.qrc)



add_executable(${PROJECT_NAME}
    ${SRC}
    ${ui_RESOURCE}
)

target_link_libraries(${PROJECT_NAME} common)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets
                                      Qt${QT_VERSION_MAJOR}::Network
                                      Qt${QT_VERSION_MAJOR}::DBus
                                      Qt${QT_VERSION_MAJOR}::Sql
                                      Qt${QT_VERSION_MAJOR}::Concurrent)

#dependencies
target_link_libraries(${PROJECT_NAME} "-lpthread -lz")

target_link_lingmosdk_libraries(${PROJECT_NAME})
target_link_KWindowSystem_libraries(${PROJECT_NAME})
target_link_qgsettings_libraries(${PROJECT_NAME})

########### install ###############
install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
file(GLOB TRANS "${PROJECT_SOURCE_DIR}/../translations/${PROJECT_NAME}/*.qm")
install(FILES ${TRANS} DESTINATION  /usr/share/${PROJECT_NAME}/translations/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/../data/lingmo-notebook DESTINATION /usr/share/lingmo-user-guide/data/guide/)
install(FILES ${PROJECT_SOURCE_DIR}/../data/${PROJECT_NAME}.desktop DESTINATION /usr/share/applications/)
