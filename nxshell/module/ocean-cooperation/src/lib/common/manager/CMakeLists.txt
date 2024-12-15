cmake_minimum_required(VERSION 3.13)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# enable qt build support
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# # set build as share library
# set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared Libs" FORCE)

project(sessionmanager VERSION 1.0.0)

if (WIN32)
    # 从定位输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY}/ocean-cooperation)
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

# if(BUILD_SHARED_LIBS)
#     target_compile_definitions(${PROJECT_NAME} PRIVATE QT_BUILD_SHARED)
#     set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
#     set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hioceann VISIBILITY_INLINES_HIOCEANN ON)
# endif()

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

#install library file
# install(TARGETS ${PROJECT_NAME}
#         LIBRARY DESTINATION ${LIB_INSTALL_DIR})

