include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(libboltfakeserver_SRCS
    fakedevice.cpp
    fakemanager.cpp
    fakeserver.cpp
)

qt_add_dbus_adaptor(
    libboltfakeserver_SRCS
    ${CMAKE_SOURCE_DIR}/src/interfaces/org.freedesktop.bolt1.device.xml
    fakedevice.h FakeDevice fakedeviceadaptor FakeDeviceAdaptor
)
qt_add_dbus_adaptor(
    libboltfakeserver_SRCS
    ${CMAKE_SOURCE_DIR}/src/interfaces/org.freedesktop.bolt1.manager.xml
    fakemanager.h FakeManager fakemanageradaptor FakeManagerAdaptor
)

add_library(libboltfakeserver STATIC ${libboltfakeserver_SRCS})
target_link_libraries(libboltfakeserver Qt::Core Qt::DBus Qt::Test)
set_target_properties(libboltfakeserver PROPERTIES OUTPUT_NAME boltfakeserver CXX_STANDARD 14)
target_include_directories(libboltfakeserver PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>;$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")

#######################################################################

set(boltfakeserver_SRCS
    main.cpp
)

add_executable(boltfakeserver ${boltfakeserver_SRCS})
target_link_libraries(boltfakeserver libboltfakeserver Qt::Core Qt::DBus)
