cmake_minimum_required(VERSION 3.7)

set(BIN_NAME deepin-system-monitor-system-server)

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
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)
find_package(PolkitQt5-1 REQUIRED)
find_package(Dtk REQUIRED COMPONENTS Core)

add_executable(${BIN_NAME}
    ${SRC_CPP}
    ${SRC_H}
)

target_include_directories(${BIN_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
  PolkitQt5-1::Agent
  Dtk::Core
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::DBus
  PolkitQt5-1::Agent
  Dtk::Core
)

# Use deepin-service-manager mange DBus servicde
install(TARGETS ${BIN_NAME} DESTINATION /usr/lib/deepin-daemon/)
install(FILES ./misc/${BIN_NAME}.json DESTINATION share/deepin-service-manager/other/)
# DBus service and conf
install(FILES ./misc/org.deepin.SystemMonitorSystemServer.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)
install(FILES ./misc/org.deepin.SystemMonitorSystemServer.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system.d)
# Systemd
install(FILES ./misc/deepin-system-monitor-system-server.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/)
# Polkit; Update translation
set(POLICY_FILE org.deepin.deepin-system-monitor-system-server.policy)
find_program(POLICY_TS_CONV deepin-policy-ts-convert REQUIRED)
add_custom_target(APP_POLICY_FILE_TEST
    ALL
    COMMAND ${POLICY_TS_CONV} ts2policy ${CMAKE_CURRENT_SOURCE_DIR}/misc/${POLICY_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/translations/policy/ ${POLICY_FILE}.tmp
    COMMAND mv ${POLICY_FILE}.tmp ${CMAKE_CURRENT_SOURCE_DIR}/misc/${POLICY_FILE}
)
install(FILES ./misc/${POLICY_FILE} DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions)
