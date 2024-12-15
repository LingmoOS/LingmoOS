cmake_minimum_required(VERSION 3.16)

set(BIN_NAME ocean-polkit-agent)

project(${BIN_NAME} CXX)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

option(USE_LINGMO_POLKIT "enable lingmo polkit" OFF)

message(STATUS "TO USE Lingmo_Polkit : ${USE_LINGMO_POLKIT}")

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

set(QT_VERSION_MAJOR 6)
set(DTK_VERSION_MAJOR 6)

include(GNUInstallDirs)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED Core Widgets DBus Concurrent LinguistTools)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED Widget Core Tools)
find_package(OCEANShell REQUIRED)

pkg_check_modules(Polkit-qt6 REQUIRED polkit-qt6-1)

qt_add_dbus_adaptor(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/adapter/org.lingmo.ocean.Polkit1.AuthAgent.xml policykitlistener.h PolicyKitListener)

if(${USE_LINGMO_POLKIT})
  add_definitions(-DUSE_LINGMO_POLKIT)
endif()

dtk_add_dbus_interfaces(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.lingmo.ocean.Accounts1.xml)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.lingmo.ocean.Accounts1.User.xml accounts1userinterface)

file(GLOB OCEAN_POLKIT_AGENT_SCRS "*.h" "*.cpp")

add_executable(${BIN_NAME}
    ${DBUS_INTERFACES}
    ${OCEAN_POLKIT_AGENT_SCRS}
    images.qrc
)

target_include_directories(${BIN_NAME} PUBLIC
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    ${Polkit-qt6_INCLUDE_DIRS}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
)

target_link_libraries(${BIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    Dde::Shell
    ${Polkit-qt6_LIBRARIES}
)

# can use qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES xx xx) from Qt6.7
set(TRANSLATION_LANGUAGES
    af am_ET ar ast az bg bn bo ca cs da de el en_AU en_GB eo es et fa fi fr gl_ES
    he hi_IN hr hu hy id it ja ko ku_IQ lo lt ml mn ms nb ne nl pl pt_BR pt ro ru
    si sk sl sq sr sv sw ta tr pam ug uk vi zh_CN zh_HK zh_TW
)

set(TRANSLATION_FILES "")
list(APPEND TRANSLATION_FILES "${PROJECT_SOURCE_DIR}/translations/ocean-polkit-agent.ts")

foreach(LANG ${TRANSLATION_LANGUAGES})
    list(APPEND TRANSLATION_FILES "${PROJECT_SOURCE_DIR}/translations/ocean-polkit-agent_${LANG}.ts")
endforeach()

# can generate ts and qm file
qt_add_translations(${BIN_NAME}
    TS_FILES ${TRANSLATION_FILES}
    SOURCES ${OCEAN_POLKIT_AGENT_SCRS}
    QM_FILES_OUTPUT_VARIABLE TR_QM_FILES
)

## qm files
install(FILES ${TR_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${BIN_NAME}/translations)

## header files
file(GLOB HEADER_FILES "agent-extension-proxy.h" "agent-extension.h")
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dpa)

# bin
install(TARGETS ${BIN_NAME} DESTINATION lib/polkit-1-ocean)
