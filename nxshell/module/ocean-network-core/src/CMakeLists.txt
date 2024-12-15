cmake_minimum_required(VERSION 3.7)

set(VERSION "3.0.0" CACHE STRING "define project version")
project(ocean-network-core6)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

file(GLOB_RECURSE SOURCEFILES "*.h" "*.cpp")




add_library(${PROJECT_NAME} SHARED ${SOURCEFILES})

find_package(Qt6 COMPONENTS Core DBus Network Gui LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Dtk6 COMPONENTS Core REQUIRED)
find_package(KF6NetworkManagerQt REQUIRED)

pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)

file(GLOB TS_FILES "translations/*.ts")

qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(networktranslations ALL DEPENDS ${QM_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    dbus
    impl
    impl/networkmanager
    impl/serviceinter
    Qt6::Core
    Qt6::DBus
    Qt6::Network
    Qt6::Gui
    Dtk6::Core
    KF6::NetworkManagerQt
    PkgConfig::LibNM
)

message(STATUS ${LibNM_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::DBus
    Qt6::Network
    Qt6::Gui
    Dtk6::Core
    KF6::NetworkManagerQt
    PkgConfig::LibNM
    udev
)

configure_file(ocean-network-core.pc.in ../ocean-network-core.pc @ONLY)

set(INTERFACEFILES
    impl/dslcontroller.h
    impl/hotspotcontroller.h
    impl/vpncontroller.h
    networkconst.h
    networkcontroller.h
    networkdetails.h
    networkdevicebase.h
    proxycontroller.h
    wireoceanvice.h
    wirelessdevice.h)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    # OUTPUT_NAME ocean-network-core
    # EXPORT_NAME Network-Core
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/ocean-network-core.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${INTERFACEFILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/liboceannetworkcore)
install(FILES ../config/org.lingmo.ocean.network.json DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/org.lingmo.ocean.network)
# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/ocean-network-core/translations)
