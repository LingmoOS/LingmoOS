cmake_minimum_required(VERSION 3.13)

set(BIN_NAME dde-polkit-agent)

project(${BIN_NAME} CXX)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

option(USE_DEEPIN_POLKIT "enable deepin polkit" OFF)

message(STATUS "TO USE Deepin_Polkit : ${USE_DEEPIN_POLKIT}")

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "-z relro -z now -z noexecstack -pie")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets DBus Concurrent REQUIRED)
find_package(DtkTools REQUIRED)

pkg_check_modules(Polkit-qt5 REQUIRED polkit-qt5-1)

function(generation_dbus_adaptor xml class_name class_file option)
    execute_process(COMMAND qdbusxml2cpp ${option} -a ${class_file} -c ${class_name} ${xml}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction(generation_dbus_adaptor)

generation_dbus_adaptor(
    ${CMAKE_SOURCE_DIR}/dbus/adapter/org.deepin.dde.Polkit1.AuthAgent.xml
    Polkit1AuthAgentAdaptor
    ${CMAKE_CURRENT_BINARY_DIR}/polkit1authagent_adaptor
    -N
)

set(DBUS_INTERFACE_DIR ${PROJECT_SOURCE_DIR}/dbus/interface)
function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND ${DTK_XML2CPP} -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${DBUS_INTERFACE_DIR})
endfunction(generation_dbus_interface)

if(${USE_DEEPIN_POLKIT})
  add_definitions(-DUSE_DEEPIN_POLKIT)
endif()

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Accounts1.User.xml
    AccountsUser
    ${DBUS_INTERFACE_DIR}/accountuser_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Accounts1.xml
    Accounts
    ${DBUS_INTERFACE_DIR}/account_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Fprintd1.Device.xml
    FprintdDevice
    ${DBUS_INTERFACE_DIR}/fprintddevice_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Fprintd1.xml
    FPrintd
    ${DBUS_INTERFACE_DIR}/fprintd_interface
)

aux_source_directory(${PROJECT_SOURCE_DIR}/dbus/interface/ DBUS_INTERFACES)

set(AuthAgent_DBUS_SCRS
    ${CMAKE_CURRENT_BINARY_DIR}/polkit1authagent_adaptor.cpp
)

add_executable(${BIN_NAME}
    policykitlistener.cpp
    AuthDialog.cpp
    main.cpp
    usersmanager.cpp
    pluginmanager.cpp
    images.qrc
    ${AuthAgent_DBUS_SCRS}
    ${DBUS_INTERFACES}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    ${Polkit-qt5_INCLUDE_DIRS}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
    ${PROJECT_SOURCE_DIR}/dbus/interface
    ${PROJECT_SOURCE_DIR}/dbus/qtdbusextended
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Polkit-qt5_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
)

## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/${BIN_NAME}/translations)

## header files
file(GLOB HEADER_FILES "agent-extension-proxy.h" "agent-extension.h")
install(FILES ${HEADER_FILES} DESTINATION include/dpa)

# bin
install(TARGETS ${BIN_NAME} DESTINATION lib/polkit-1-dde)
