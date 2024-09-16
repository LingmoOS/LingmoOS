cmake_minimum_required(VERSION 3.0)

set(BIN_NAME "deepin-diskencrypt-service")
project(${BIN_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Concurrent DBus)
find_package(deepin-qdbus-service REQUIRED)
find_package(dfm-framework REQUIRED)
find_package(dfm-mount REQUIRED)
find_package(dfm-base REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CryptSetup REQUIRED libcryptsetup)

add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")

# generate dbus xml and adaptor
# DBus: DiskEncryptDBus
# qt5_generate_dbus_interface(
#    diskencryptdbus.h
#    diskencryptdbus.xml
#    OPTIONS -M -S
#)
qt5_add_dbus_adaptor(ADAPTER_SRC
                     diskencryptdbus.xml
                     diskencryptdbus.h
                     DiskEncryptDBus)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${BIN_NAME}
    ${SRC}
    ${ADAPTER_SRC}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Core
    Qt5::Concurrent
    Qt5::DBus
    ${CryptSetup_LIBRARIES}
    ${dfm-framework_LIBRARIES}
    ${dfm-mount_LIBRARIES}
    ${dfm-base_LIBRARIES}
    ${deepin-qdbus-service_LIBRARIES}
)

target_include_directories(${BIN_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${dfm-framework_INCLUDE_DIRS}
    ${CryptSetup_INCLUDE_DIRS}
    ${dfm-mount_INCLUDE_DIRS}
    ${deepin-qdbus-service_INCLUDE_DIR}
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.filemanager.diskencrypt.conf DESTINATION share/dbus-1/system.d/)
install(FILES org.deepin.Filemanager.DiskEncrypt.service DESTINATION share/dbus-1/system-services/)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES ${CMAKE_SOURCE_DIR}/assets/polkit/policy/org.deepin.filemanager.diskencrypt.policy
    DESTINATION ${PolicyDir})
install(FILES ${CMAKE_SOURCE_DIR}/assets/polkit/rules/99-dde-file-manager-encrypt.pkla
    DESTINATION /etc/polkit-1/localauthority/10-vendor.d)
