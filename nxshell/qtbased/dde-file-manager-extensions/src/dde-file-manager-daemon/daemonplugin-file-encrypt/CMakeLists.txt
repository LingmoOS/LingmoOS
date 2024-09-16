cmake_minimum_required(VERSION 3.0)

project(daemonplugin-file-encrypt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Concurrent DBus)
find_package(dfm-framework REQUIRED)
find_package(dfm-mount REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CryptSetup REQUIRED libcryptsetup)

# generate dbus xml and adaptor
execute_process(COMMAND qdbuscpp2xml
        -M -S diskencryptdbus.h
        -o diskencryptdbus.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus
)
execute_process(COMMAND qdbusxml2cpp
        -i diskencryptdbus.h
        -c DiskEncryptDBusAdaptor
        -l daemonplugin_file_encrypt::DiskEncryptDBus
        -a diskencryptdbus_adaptor
        diskencryptdbus.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus
)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${PROJECT_NAME} SHARED
  ${SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Concurrent
    Qt5::DBus
    ${CryptSetup_LIBRARIES}
    ${dfm-framework_LIBRARIES}
    ${dfm-mount_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${dfm-framework_INCLUDE_DIRS}
    ${CryptSetup_INCLUDE_DIRS}
    ${dfm-mount_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE DFMPLUGIN_DISK_ENCRYPT_LIBRARY)
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_DAEMON_EDGE_DIR})
message(">>>> install ${PROJECT_NAME} into ${DFM_PLUGIN_DAEMON_EDGE_DIR}")

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES ${CMAKE_SOURCE_DIR}/assets/polkit/policy/com.deepin.filemanager.daemon.diskencrypt.policy
    DESTINATION ${PolicyDir})
install(FILES ${CMAKE_SOURCE_DIR}/assets/polkit/rules/99-dde-file-manager-encrypt.pkla
    DESTINATION /etc/polkit-1/localauthority/10-vendor.d)

