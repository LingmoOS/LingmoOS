cmake_minimum_required(VERSION 3.13.4)
project(ocean-session-shell C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -pthread -Wl,--as-need -Wl,-E")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# 增加安全编译参数
set(SECURITY_COMPILE, "-fstack-protector-all -fPIC")
set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} ${SECURITY_COMPILE}")
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} ${SECURITY_COMPILE}")

# coverage option
# cmake -DENABLE_COVERAGE=ON ..
OPTION (ENABLE_COVERAGE "Use gcov" OFF)
option (CONTRAST_BACKGROUND "Fill background using high-contrast color for debug" OFF)
MESSAGE(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
if (ENABLE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    #SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
endif()

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
include(GNUInstallDirs)

# Find the library
find_package(PAM REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCMake  REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Widgets
    Concurrent
    X11Extras
    DBus
    Xml
    Svg
    Network
REQUIRED)
find_package(DtkTools REQUIRED)

pkg_check_modules(XCB_EWMH REQUIRED xcb-ewmh x11 xi xcursor xfixes xrandr xext xtst)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(Greeter REQUIRED liblightdm-qt5-3)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Xml
    Qt5::Svg
)

set(XML2CPP_DIR ${PROJECT_SOURCE_DIR}/src/global_util/dbus)

function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND ${DTK_XML2CPP} -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${XML2CPP_DIR})
endfunction(generation_dbus_interface)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.Authenticate1.xml
    Authenticate
    ${XML2CPP_DIR}/authenticate_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/com.huawei.switchos.xml
    HuaWeiSwitchOS
    ${XML2CPP_DIR}/huaweiswitchos_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.Accounts1.xml
    Accounts
    ${XML2CPP_DIR}/accounts_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.Accounts1.User.xml
    AccountsUser
    ${XML2CPP_DIR}/accountsuser_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.ImageEffect1.xml
    ImageEffect
    ${XML2CPP_DIR}/imageeffect_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.Logined.xml
    Logined
    ${XML2CPP_DIR}/logined_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.PowerManager1.xml
    PowerManager
    ${XML2CPP_DIR}/powermanager_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.SystemPower1.xml
    SystemPower
    ${XML2CPP_DIR}/systempower_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/com.lingmo.wm.xml
    wm
    ${XML2CPP_DIR}/wm_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.Authenticate1.Session2.xml
    AuthenticateSession2
    ${XML2CPP_DIR}/authenticatesession2_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.SessionManager1.xml
    SessionManager
    ${XML2CPP_DIR}/sessionmanager_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.SoundThemePlayer1.xml
    SoundThemePlayer
    ${XML2CPP_DIR}/soundthemeplayer_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.lingmo.ocean.SystemDisplay1.xml
    SystemDisplay
    ${XML2CPP_DIR}/systemdisplay_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.freedesktop.DBus.xml
    FreedesktopDBus
    ${XML2CPP_DIR}/freedesktopdbus_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.freedesktop.login1.session.self.xml
    Login1SessionSelf
    ${XML2CPP_DIR}/login1sessionself_interface
)

include_directories(${PROJECT_SOURCE_DIR}/src/global_util)
include_directories(${PROJECT_SOURCE_DIR}/src/global_util/dbus)
include_directories(${PROJECT_SOURCE_DIR}/src/global_util/keyboardmonitor)
include_directories(${PROJECT_SOURCE_DIR}/src/widgets)
include_directories(${PROJECT_SOURCE_DIR}/src/session-widgets)
include_directories(${PROJECT_SOURCE_DIR}/src/libocean-auth)
include_directories(${PROJECT_SOURCE_DIR}/interface)
include_directories(${PROJECT_SOURCE_DIR}/src/global_util/dbus/types)
include_directories(${DTKCORE_INCLUDE_DIRS})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/global_util GLOBAL_UTILS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/global_util/dbus GLOBAL_UTILS_DBUS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/global_util/keyboardmonitor GLOBAL_UTILS_KEYBOARDMONITOR)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/widgets WIDGETS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/session-widgets SESSION_WIDGETS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/libocean-auth AUTHENTICATE)
aux_source_directory(${PROJECT_SOURCE_DIR}/interface INTERFACE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/global_util/dbus/types DBUS_DATA_TYPES)

set(QRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc
    ${PROJECT_SOURCE_DIR}/src/widgets/widgetsimages.qrc
    ${PROJECT_SOURCE_DIR}/src/widgets/widgetstheme.qrc
)

set(LOCK_SRCS
    ${transaction_DBUS_SCRS}
    ${GLOBAL_UTILS}
    ${GLOBAL_UTILS_DBUS}
    ${GLOBAL_UTILS_KEYBOARDMONITOR}
    ${WIDGETS}
    ${SESSION_WIDGETS}
    ${AUTHENTICATE}
    ${INTERFACE}
    ${QT_DBUS_EXTENDED}
    ${DBUS_DATA_TYPES}
    src/app/ocean-lock.cpp
    src/ocean-lock/lockframe.cpp
    src/ocean-lock/lockworker.cpp
    src/ocean-lock/dbus/dbuslockagent.cpp
    src/ocean-lock/dbus/dbuslockfrontservice.cpp
    src/ocean-lock/dbus/dbusshutdownagent.cpp
    src/ocean-lock/dbus/dbusshutdownfrontservice.cpp
)
add_executable(ocean-lock
    ${LOCK_SRCS}
    ${QRCS}
)
target_include_directories(ocean-lock PUBLIC
    ${PAM_INCLUDE_DIR}
    ${DTKWIDGET_INCLUDE_DIR}
    ${DTKCORE_INCLUDE_DIR}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/ocean-lock
    ${PROJECT_SOURCE_DIR}/src/ocean-lock/dbus
    ${Greeter_INCLUDE_DIRS}
)
target_link_libraries(ocean-lock PRIVATE
    ${Qt_LIBS}
    ${PAM_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Greeter_LIBRARIES}
    ${LINGMO_PW_CHECK}
)

set(LINGMO_PW_CHECK liblingmo_pw_check.so)

set(GREETER_SRCS
    ${transaction_DBUS_SCRS}
    ${GLOBAL_UTILS}
    ${GLOBAL_UTILS_DBUS}
    ${GLOBAL_UTILS_KEYBOARDMONITOR}
    ${WIDGETS}
    ${SESSION_WIDGETS}
    ${AUTHENTICATE}
    ${INTERFACE}
    ${DBUS_DATA_TYPES}
    src/app/lightdm-lingmo-greeter.cpp
    src/lightdm-lingmo-greeter/loginframe.cpp
    src/lightdm-lingmo-greeter/greeterworker.cpp
    src/lightdm-lingmo-greeter/logincontent.cpp
    src/lightdm-lingmo-greeter/logintipswindow.cpp
    src/lightdm-lingmo-greeter/pwqualitymanager.cpp
    src/lightdm-lingmo-greeter/passwordlevelwidget.cpp
    src/lightdm-lingmo-greeter/changepasswordwidget.cpp
)
add_executable(lightdm-lingmo-greeter
    ${GREETER_SRCS}
    ${QRCS}
)

target_include_directories(lightdm-lingmo-greeter PUBLIC
    ${PAM_INCLUDE_DIR}
    ${DTKWIDGET_INCLUDE_DIR}
    ${DTKCORE_INCLUDE_DIR}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${Greeter_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter
)
target_compile_definitions(lightdm-lingmo-greeter
    PRIVATE ENABLE_SESSION
)

if (WAIT_LINGMO_ACCOUNTS_SERVICE)
    target_compile_definitions(lightdm-lingmo-greeter
        PUBLIC ENABLE_WAITING_ACCOUNTS_SERVICE
    )
endif()

target_link_libraries(lightdm-lingmo-greeter PRIVATE
    ${Qt_LIBS}
    ${PAM_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Greeter_LIBRARIES}
    ${LINGMO_PW_CHECK}
)

if(CONTRAST_BACKGROUND)
    target_compile_definitions(ocean-lock PRIVATE CONTRAST_BACKGROUND)
    target_compile_definitions(lightdm-lingmo-greeter PRIVATE CONTRAST_BACKGROUND)
endif()

# plugins
add_subdirectory(modules)

add_subdirectory(tests)

if(NOT DSS_CHECK_ACCESSIBILITY)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/src/global_util/accessibilitycheckerex.h 
    TARGET_DIRECTORY ocean-lock ocean-lock-test lightdm-lingmo-greeter lightdm-lingmo-greeter-test
    PROPERTIES SKIP_AUTOMOC ON)
else()
    add_compile_definitions(-DDSS_CHECK_ACCESSIBILITY)
endif()

# bin
install(TARGETS ocean-lock lightdm-lingmo-greeter DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES files/lingmo-greeter DESTINATION ${CMAKE_INSTALL_BINDIR})

file(GLOB SCRIPTS  scripts/*)
install(PROGRAMS ${SCRIPTS} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/lingmo/greeters.d)

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/ocean-session-shell/translations)
install(FILES qt-theme.ini DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/lightdm/lingmo)

install(FILES files/lightdm-lingmo-greeter.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/xgreeters/)
install(FILES files/ocean-lock.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/)
install(FILES files/ocean-session-shell.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/ocean-session-shell/)
install(FILES files/lightdm-lingmo-greeter.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/lingmo-authentication/privileges/)

# services
file(GLOB SERVICES files/*.service)
install(FILES ${SERVICES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

#schemas
install(FILES xml/com.lingmo.ocean.session-shell.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)

# dev
file(GLOB INTERFACE interface/*.h)
install(FILES ${INTERFACE} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ocean-session-shell)

file(GLOB CMAKE_CONFIG cmake/*.cmake)
install(FILES ${CMAKE_CONFIG} DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DdeSessionShell)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/DdeSessionShellConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DdeSessionShellConfig.cmake
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DdeSessionShellConfig.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DdeSessionShell)

# pam
file(GLOB PAM_CONFIG files/pam.d/*)
install(FILES ${PAM_CONFIG} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/pam.d/)

# config
dconfig_meta_files(APPID org.lingmo.ocean.lock FILES configs/org.lingmo.ocean.lock.json)
dconfig_meta_files(APPID org.lingmo.ocean.lightdm-lingmo-greeter FILES configs/org.lingmo.ocean.lightdm-lingmo-greeter.json)
