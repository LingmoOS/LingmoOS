set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH $ENV{Qt5_DIR})

file(GLOB_RECURSE SOURCEFILES "*.h" "*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCEFILES})

find_package(Qt5 COMPONENTS Core DBus Network REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)

pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)

get_target_property(KF5_QT_INCLUDE_DIRS KF5::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

set(TRY_KF5_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

if (NOT KF5_QT_LIBRARIES)
    find_library(KF5_QT_LIBRARIES libKF5NetworkManagerQt.so)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    realize
    ${KF5_QT_INCLUDE_DIRS}
    ${NETINTERFACEINCLUDE})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    PkgConfig::QGSettings
    ${KF5_QT_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)

configure_file(dde-network-core.pc.in ../dde-network-core.pc @ONLY)

set(INTERFACEFILES
    dslcontroller.h
    hotspotcontroller.h
    networkconst.h
    networkcontroller.h
    networkdetails.h
    networkdevicebase.h
    proxycontroller.h
    vpncontroller.h
    wireddevice.h
    wirelessdevice.h)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/dde-network-core.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${INTERFACEFILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libddenetworkcore)
install(FILES ../config/org.deepin.dde.network.json DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/org.deepin.dde.network)
