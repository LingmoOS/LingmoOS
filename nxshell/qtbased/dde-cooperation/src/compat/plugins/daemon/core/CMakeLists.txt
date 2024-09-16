cmake_minimum_required(VERSION 3.13)

project(daemon-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB PLUGIN_FILES
    "${COMPAT_ROOT_DIR}/common/commonutils.h"
    "${COMPAT_ROOT_DIR}/common/commonutils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Concurrent)

find_package(Threads REQUIRED)

include_directories(include)

include_directories(${COMPAT_ROOT_DIR}/protocol)

add_library(${PROJECT_NAME}
    SHARED
    ${PLUGIN_FILES}
    ${QRC_RES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src/
)


target_link_libraries(${PROJECT_NAME}
    deepin_cross::framework
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
    uniapi
    co
    CuteIPC
)

#System libraries
if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 wsock32)
endif ()
target_link_libraries(${PROJECT_NAME} Threads::Threads)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DEEPIN_DAEMON_PLUGIN_DIR}
)

