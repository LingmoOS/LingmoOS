include_directories(
    ${CMAKE_SOURCE_DIR}/src/settings
)

set(fakeNetwork_SRCS
    accesspoint.cpp
    activeconnection.cpp
    connection.cpp
    device.cpp
    fakenetwork.cpp
    settings.cpp
    wireddevice.cpp
    wirelessdevice.cpp
)

add_library(fakeNetwork STATIC ${fakeNetwork_SRCS})

target_link_libraries(fakeNetwork
    KF6NetworkManagerQt_static
    Qt6::Core
    Qt6::Network
    Qt6::DBus
    PkgConfig::NetworkManager)
