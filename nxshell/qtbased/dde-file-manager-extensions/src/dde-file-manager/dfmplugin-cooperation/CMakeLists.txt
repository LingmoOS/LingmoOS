cmake_minimum_required(VERSION 3.13)

project(dfmplugin-cooperation)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB PLUGIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/reportlog/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/reportlog/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)
find_package(dfm-base REQUIRED)
find_package(dfm-framework REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${PLUGIN_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src/
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

# 公共依赖库，添加到这里
target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${DtkWidget_LIBRARIES}
    ${dfm-base_LIBRARIES}
    ${dfm-framework_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_EDGE_DIR}
)

execute_process(
    COMMAND lupdate
        ./
        -ts
        -no-obsolete
        ${CMAKE_SOURCE_DIR}/translations/cooperation-transfer.ts
    COMMAND lupdate
        ./
        -ts
        -no-obsolete
        ${CMAKE_SOURCE_DIR}/translations/cooperation-transfer_zh_CN.ts

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 查找匹配 cooperation-transfer*.ts 的文件列表
file(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/translations/cooperation-transfer*.ts)

# 添加 lrelease 命令，传递 TS_FILES 列表
foreach(TS_FILE ${TS_FILES})
    execute_process(
       COMMAND lrelease ${TS_FILE}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/translations
    DESTINATION share/dde-file-manager
    FILES_MATCHING PATTERN "*.qm")
