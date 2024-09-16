set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.9)

set(DAEMON_NAME "deepin-system-upgrade-daemon")
set(GOPATH_DIR "gopath")
set(GO_CODE "/usr/share/gocode")
set(PROG_UPGRADE ${DAEMON_NAME})

if(EXISTS ${GOPATH_DIR}/src/${DAEMON_NAME})
    exec_program(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/${GOPATH_DIR})
endif()

    execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/${GOPATH_DIR}/src/${DAEMON_NAME})
    execute_process(COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/pkg ${CMAKE_CURRENT_SOURCE_DIR}/${GOPATH_DIR}/src/${DAEMON_NAME})
    execute_process(COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/cmd ${CMAKE_CURRENT_SOURCE_DIR}/${GOPATH_DIR}/src/${DAEMON_NAME})


string(CONCAT GOPATH_LIST ${CMAKE_CURRENT_SOURCE_DIR}/${GOPATH_DIR} ":" ${GO_CODE})

set(GOPATH
    $ENV{GOPATH}
    ${GOPATH_LIST}
)

set(CMAKE_GO_FLAGS -gcflags \"-N -l\")

project(${PROJECT_NAME} Go) # select GO compile

include(cmake/golang.cmake)

set(DAEMON_SRCS
    "cmd/${DAEMON_NAME}/main.go"
)

add_go_executable(${DAEMON_NAME} ${DAEMON_SRCS})

# 设置安装路径的前缀(默认为"/usr")
set(CMAKE_INSTALL_PREFIX "/usr")

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${DAEMON_NAME} DESTINATION sbin)

install(FILES configs/dbus/org.deepin.SystemUpgrade1.conf DESTINATION share/dbus-1/system.d)
install(FILES configs/dbus/org.deepin.SystemUpgrade1.service DESTINATION share/dbus-1/system-services)

install(FILES configs/backup.yaml DESTINATION /etc/deepin-system-upgrade)
install(FILES configs/config.yaml DESTINATION /etc/deepin-system-upgrade)
install(FILES configs/upgrade.yaml DESTINATION /etc/deepin-system-upgrade)