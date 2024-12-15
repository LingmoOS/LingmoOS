cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(LIB_NAME semanticparser)

find_package(Qt5 COMPONENTS Gui DBus REQUIRED)
find_package(PkgConfig REQUIRED)


FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.lingmo.ai.daemon.VectorIndex.xml vectorindex)
qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.lingmo.ai.daemon.QueryLang.xml querylang)

add_library(${LIB_NAME} STATIC ${SRC_FILES} ${IFS_SRC})

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    ${LIB_NAME}
    Qt5::Gui
)
