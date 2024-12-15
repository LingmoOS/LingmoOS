cmake_minimum_required(VERSION 3.16)

set(BIN_NAME ocean-clipboard)

project(${BIN_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

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
endif ()

set(QT_VERSION_MAJOR 6)
set(DTK_VERSION_MAJOR 6)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget Core Tools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets DBus WaylandClient Test Concurrent LinguistTools REQUIRED)
find_package(OCEANShell REQUIRED)
find_package(DdeTrayLoader REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-qt6)

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

include_directories(
    ocean-clipboard
    ocean-clipboard/dbus
    ocean-clipboard/displaymanager
    ocean-clipboardloader
)

#----------------------------ocean-clipboard------------------------------

qt_add_dbus_adaptor(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/ocean-clipboard/org.lingmo.ocean.Clipboard1.xml mainwindow.h MainWindow)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.lingmo.ocean.Display1.Monitor.xml display1monitorinterface)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.lingmo.ocean.Display1.xml display1interface)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.lingmo.ocean.daemon.Dock1.xml dock1interface)

include_directories(
    ${PROJECT_SOURCE_DIR}/dbus/interface
    ${PROJECT_SOURCE_DIR}/dbus/types
    ${PROJECT_SOURCE_DIR}/dbus
)

aux_source_directory(dbus/types DBUS_TYPES)

file(GLOB_RECURSE Clipboard_SCRS
    "ocean-clipboard/*.h"
    "ocean-clipboard/*.cpp"
)
list(REMOVE_ITEM Clipboard_SCRS "${CMAKE_SOURCE_DIR}/ocean-clipboard/main.cpp")

add_executable(${BIN_NAME}
    ${Clipboard_SCRS}
    ${Clipboard_DBUS_SCRS}
    ocean-clipboard/main.cpp
    ${DBUS_INTERFACES}
    ${DBUS_TYPES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    PkgConfig::GIO
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    Dde::Shell
)

# can use qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES xx xx) from Qt6.7
set(TRANSLATION_LANGUAGES
    af am_ET ar ast az bg bn bo ca cs da de el_GR el en_AU en_GB eo es_419 es
    et fa fi fr gl_ES he hi_IN hi hr hu hy id it ja kab ko ku_IQ ku ky@Arab
    lt lv ml mn ms nb ne nl pa pl pt_BR pt ro ru si sk sl sq sr sv sw ta tr
    ug uk vi zh_CN zh_HK zh_TW
)

set(TRANSLATION_FILES "")
list(APPEND TRANSLATION_FILES "${PROJECT_SOURCE_DIR}/translations/ocean-clipboard.ts")

foreach(LANG ${TRANSLATION_LANGUAGES})
    list(APPEND TRANSLATION_FILES "${PROJECT_SOURCE_DIR}/translations/ocean-clipboard_${LANG}.ts")
endforeach()

# can generate ts and qm file
qt_add_translations(${BIN_NAME}
    TS_FILES ${TRANSLATION_FILES}
    SOURCES ${Clipboard_SCRS}
    QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
)

macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SYSTEMD_USER_UNIT_DIR}/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION ${SYSTEMD_USER_UNIT_DIR}/${wantsdir}/)
endmacro(install_symlink)

## qm files
install(FILES ${TRANSLATED_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${BIN_NAME}/translations)

configure_file(
    misc/ocean-clipboard.service.in
    ocean-clipboard.service
    @ONLY
)

configure_file(
    misc/org.lingmo.ocean.Clipboard1.service.in
    org.lingmo.ocean.Clipboard1.service
    @ONLY
)

## dbus service
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.ocean.Clipboard1.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ocean-clipboard.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

install_symlink(ocean-clipboard.service ocean-session-initialized.target.wants)

## bin
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

#----------------------------ocean-clipboard-daemon------------------------------
set(BIN_NAME ocean-clipboard-daemon)

file(GLOB_RECURSE ocean-clipboard-daemon_SCRS
    "ocean-clipboard-daemon/*.h"
    "ocean-clipboard-daemon/*.cpp"
)

add_executable(${BIN_NAME}
    ${ocean-clipboard-daemon_SCRS}
)

qt_generate_wayland_protocol_client_sources(${BIN_NAME} FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ocean-clipboard-daemon/protocol/wlr-data-control-unstable-v1.xml)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WaylandClient
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    Dtk${DTK_VERSION_MAJOR}::Core
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(
    misc/org.lingmo.ocean.ClipboardLoader1.service.in
    org.lingmo.ocean.ClipboardLoader1.service
    @ONLY
)

configure_file(
    misc/org.lingmo.ocean.daemon.Clipboard1.service.in
    org.lingmo.ocean.daemon.Clipboard1.service
    @ONLY
)
configure_file(
    misc/ocean-clipboard-daemon.service.in
    ocean-clipboard-daemon.service
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.ocean.ClipboardLoader1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.ocean.daemon.Clipboard1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ocean-clipboard-daemon.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

#----------------------------ut-ocean-clipboard------------------------------
set(UT_BIN_NAME ut-ocean-clipboard)

file(GLOB_RECURSE ut_Clipboard_SCRS
    "tests/ocean-clipboard/*.h"
    "tests/ocean-clipboard/*.cpp"
    "tests/ocean-clipboard/*.qrc"
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
    Dtk${DTK_VERSION_MAJOR}::Widget
    PkgConfig::GIO
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Dde::Shell
    -lpthread
    -lgcov
    -lgtest
)

#--------------------------dock-plugin---------------------------
set(PLUGIN_NAME dock-clipboard-plugin)

file(GLOB_RECURSE SRCS
    "dock-clipboard-plugin/*.h"
    "dock-clipboard-plugin/*.cpp"
    "dock-clipboard-plugin/widget/*.h"
    "dock-clipboard-plugin/widget/*.cpp"
)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} dock-clipboard-plugin/resources/clipboard.qrc)
target_include_directories(${PLUGIN_NAME} PUBLIC
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Core
    ${OBJECT_BINARY_DIR}
    ${OCEAN_DOCK_INCLUDE_DIR}
    dock-clipboard-plugin/widget/
)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins)
install(FILES "dock-clipboard-plugin/resources/clipboard.svg" DESTINATION share/ocean-dock/icons/oceanui-setting)
