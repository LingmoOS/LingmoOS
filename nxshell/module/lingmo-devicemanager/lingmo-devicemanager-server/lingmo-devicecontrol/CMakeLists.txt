cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "lingmo-devicecontrol")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lingmo-devicemanager/src/DDLog)
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus Network Sql REQUIRED)
find_package(DtkCore REQUIRED)
find_package(PolkitQt5-1 REQUIRED)
find_package(lingmo-qdbus-service REQUIRED)

if(NOT DISABLE_DRIVER)
    find_package(QApt REQUIRED)
    PKG_SEARCH_MODULE(kmod REQUIRED libkmod IMPORTED_TARGET)
endif()

add_executable(${BIN_NAME}
    ${SRC_CPP}
    ${SRC_H}
)

include_directories("/usr/include/cups/")

add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/lingmo-service-manager/")

target_include_directories(${BIN_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
  Qt5::Network
  Qt5::Sql
  ${lingmo-qdbus-service_INCLUDE_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    Qt5::Sql
    PolkitQt5-1::Agent
    ${DtkCore_LIBRARIES}
    ${lingmo-qdbus-service_LIBRARIES}
    cups
    )
if(NOT DISABLE_DRIVER)
    target_link_libraries(${BIN_NAME} PRIVATE
        kmod
        QApt
        )
endif()

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${BIN_NAME}.json DESTINATION share/lingmo-service-manager/other/)
install(FILES org.lingmo.devicecontrol.conf DESTINATION share/dbus-1/system.d/)
install(FILES org.lingmo.DeviceControl.service DESTINATION share/dbus-1/system-services/)
install(FILES lingmo-devicecontrol.service DESTINATION lib/systemd/system/)
# 添加安全加固策略
install(DIRECTORY lingmo-service-group@.service.d DESTINATION /etc/systemd/system/)