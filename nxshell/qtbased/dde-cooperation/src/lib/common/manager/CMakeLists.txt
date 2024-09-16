cmake_minimum_required(VERSION 3.13)

project(sessionmanager)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# enable qt build support
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (WIN32)
    # 从定位输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY}/dde-cooperation)
endif()

FILE(GLOB SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/common/log.h"
    "${CMAKE_SOURCE_DIR}/src/common/constant.h"
    "${CMAKE_SOURCE_DIR}/src/common/logger.h"
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/commonutils.h"
    "${CMAKE_SOURCE_DIR}/src/common/commonutils.cpp"
    *.h
    *.cpp
    )

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)

list(APPEND LINKLIBS cpplogging)
list(APPEND LINKLIBS session)
list(APPEND LINKLIBS httpweb)

add_library(${PROJECT_NAME} ${SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src/
)

# 公共依赖库，添加到这里
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Network
    ${LINKLIBS}
)

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# #install library file
# install(TARGETS ${PROJECT_NAME}
#         RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
#         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
#         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
