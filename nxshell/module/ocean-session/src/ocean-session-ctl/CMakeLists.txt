#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
qt5_add_dbus_interface(DBUS_SOURCES
    ../../dbus/adaptor/org.lingmo.ocean.Session1.xml
    Session
)

add_executable(ocean-session-ctl
  main.cpp
  ${DBUS_SOURCES}
  ../../dbus/adaptor/org.lingmo.ocean.Session1.xml
)
target_link_libraries(ocean-session-ctl
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS ocean-session-ctl DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
