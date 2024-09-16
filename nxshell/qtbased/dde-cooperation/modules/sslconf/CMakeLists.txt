cmake_minimum_required(VERSION 3.13)

project(sslconf C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_CXX_STANDARD 17)

# Module subdirectory
set(BARRIER_LIB_DIR "${CMAKE_SOURCE_DIR}/3rdparty/barrier/src/lib")

if (UNIX)
    set(PLATNAME unix)
    add_definitions(-DSYSAPI_UNIX=1)
else()
    set(PLATNAME win32)
    add_definitions(-DSYSAPI_WIN32=1)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D _BIND_TO_CURRENT_VCLIBS_VERSION=1")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /O2 /Ob2")
endif()

# Module library
file(GLOB SOURCE_FILES
    "${BARRIER_LIB_DIR}/base/finally.h"
    "${BARRIER_LIB_DIR}/base/String.h"
    "${BARRIER_LIB_DIR}/base/String.cpp"
    "${BARRIER_LIB_DIR}/io/filesystem.h"
    "${BARRIER_LIB_DIR}/io/filesystem.cpp"
    "${BARRIER_LIB_DIR}/common/DataDirectories.h"
    "${BARRIER_LIB_DIR}/common/${PLATNAME}/*.h"
    "${BARRIER_LIB_DIR}/common/${PLATNAME}/*.cpp"
    "${BARRIER_LIB_DIR}/common/DataDirectories_static.cpp"
    "${BARRIER_LIB_DIR}/net/FingerprintData.h"
    "${BARRIER_LIB_DIR}/net/FingerprintData.cpp"
    "${BARRIER_LIB_DIR}/net/FingerprintDatabase.h"
    "${BARRIER_LIB_DIR}/net/FingerprintDatabase.cpp"
    "${BARRIER_LIB_DIR}/net/SecureUtils.h"
    "${BARRIER_LIB_DIR}/net/SecureUtils.cpp"
    "*.h"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} -Wno-shadow")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${BARRIER_LIB_DIR}" PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
