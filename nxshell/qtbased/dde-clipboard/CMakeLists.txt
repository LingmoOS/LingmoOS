cmake_minimum_required(VERSION 3.16)

set(BIN_NAME dde-clipboard)

project(${BIN_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
endif()

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Dtk COMPONENTS Widget Core REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets DBus Test Concurrent REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(DWayland REQUIRED)

pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-qt)

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

find_package(DtkTools REQUIRED)
function(generation_dbus_adaptor xml class_name class_file option)
    execute_process(COMMAND qdbusxml2cpp ${option} -a ${class_file} -c ${class_name} ${xml}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction(generation_dbus_adaptor)

set(DBUS_INTERFACE ${PROJECT_SOURCE_DIR}/dbus/interface)
function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND ${DTK_XML2CPP} -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${DBUS_INTERFACE})
endfunction(generation_dbus_interface)

include_directories(
    dde-clipboard
    dde-clipboard/dbus
    dde-clipboard/displaymanager
    dde-clipboardloader
)

#----------------------------dde-clipboard------------------------------
generation_dbus_adaptor(
    ${CMAKE_SOURCE_DIR}/dde-clipboard/org.deepin.dde.Clipboard1.xml
    ClipboardAdaptor
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor
    -N
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Display1.Monitor.xml
    Monitor
    ${DBUS_INTERFACE}/monitor_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Display1.xml
    Display1
    ${DBUS_INTERFACE}/display_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.daemon.Dock1.xml
    Dock1
    ${DBUS_INTERFACE}/dock_interface
)

include_directories(
    ${PROJECT_SOURCE_DIR}/dbus/interface
    ${PROJECT_SOURCE_DIR}/dbus/types
    ${PROJECT_SOURCE_DIR}/dbus
)

aux_source_directory(dbus/types DBUS_TYPES)
aux_source_directory(dbus/interface DBUS_INTERFACES)

set(Clipboard_DBUS_SCRS
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor.h
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor.cpp
)

file(GLOB_RECURSE Clipboard_SCRS
    "dde-clipboard/*.h"
    "dde-clipboard/*.cpp"
)
list(REMOVE_ITEM Clipboard_SCRS "${CMAKE_SOURCE_DIR}/dde-clipboard/main.cpp")

add_executable(${BIN_NAME}
    ${Clipboard_SCRS}
    ${Clipboard_DBUS_SCRS}
    dde-clipboard/main.cpp
    ${DBUS_INTERFACES}
    ${DBUS_TYPES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Dtk::Widget
    Dtk::Core
    PkgConfig::GIO
    Qt5::Core
    Qt5::Widgets
    Qt5::GuiPrivate
    Qt5::Gui
    Qt5::DBus
)

macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SYSTEMD_USER_UNIT_DIR}/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION ${SYSTEMD_USER_UNIT_DIR}/${wantsdir}/)
endmacro(install_symlink)

## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${BIN_NAME}/translations)

configure_file(
    misc/dde-clipboard.service.in
    dde-clipboard.service
    @ONLY
)

configure_file(
    misc/org.deepin.dde.Clipboard1.service.in
    org.deepin.dde.Clipboard1.service
    @ONLY
)

## dbus service
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.Clipboard1.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-clipboard.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

install_symlink(dde-clipboard.service dde-session-initialized.target.wants)

## bin
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

#----------------------------dde-clipboard-daemon------------------------------
set(BIN_NAME dde-clipboard-daemon)

file(GLOB_RECURSE dde-clipboard-daemon_SCRS
        "dde-clipboard-daemon/*.h"
        "dde-clipboard-daemon/*.cpp"
)

add_executable(${BIN_NAME}
    ${dde-clipboard-daemon_SCRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Gui
  Qt5::DBus
  Qt5::Widgets
  Dtk::Core
  Qt5::Concurrent
  DWaylandClient
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(
    misc/org.deepin.dde.ClipboardLoader1.service.in
    org.deepin.dde.ClipboardLoader1.service
    @ONLY
)

configure_file(
    misc/org.deepin.dde.daemon.Clipboard1.service.in
    org.deepin.dde.daemon.Clipboard1.service
    @ONLY
)
configure_file(
    misc/dde-clipboard-daemon.service.in
    dde-clipboard-daemon.service
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.ClipboardLoader1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.daemon.Clipboard1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-clipboard-daemon.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

#----------------------------ut-dde-clipboard------------------------------
set(UT_BIN_NAME ut-dde-clipboard)

file(GLOB_RECURSE ut_Clipboard_SCRS
    "tests/dde-clipboard/*.h"
    "tests/dde-clipboard/*.cpp"
    "tests/dde-clipboard/*.qrc"
)

add_executable(${UT_BIN_NAME}
    ${Clipboard_SCRS}
    ${Clipboard_DBUS_SCRS}
    ${ut_Clipboard_SCRS}
    ${DBUS_INTERFACES}
    ${DBUS_TYPES}
)
# 用于测试覆盖率的编译条件
target_compile_options(${UT_BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

target_link_libraries(${UT_BIN_NAME} PRIVATE
    Dtk::Widget
    PkgConfig::GIO
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Test
    Qt5::GuiPrivate
    -lpthread
    -lgcov
    -lgtest
)
