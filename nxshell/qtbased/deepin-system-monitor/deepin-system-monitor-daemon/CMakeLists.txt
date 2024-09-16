cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME deepin-system-monitor)
set(BIN_NAME ${PROJECT_NAME}-daemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置包含头文件的时候不用包含路径 begin ****************************************************************************************
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)
find_package(DtkCore CONFIG REQUIRED)

qt5_add_resources(RESOURCES_FILE assets/${BIN_NAME}.qrc)

add_library(${BIN_NAME} MODULE
    ${SRC_CPP}
    ${SRC_H}
    ${RESOURCES_FILE}
)

target_include_directories(${BIN_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
  ${DtkCore_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::DBus
  ${DtkCore_LIBRARIES}
)

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-Map=deepin-system-monitor-daemon.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-Map=deepin-system-monitor-daemon.map")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/user/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/${BIN_NAME}.json ${CMAKE_BINARY_DIR}/share/system/)
else()
    install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
    install(FILES ${BIN_NAME}.json DESTINATION share/deepin-service-manager/user/)
endif()
