set(CMAKE_AUTOMOC ON)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

add_executable(lingmo-search-systemdbus
    main.cpp
    sysdbusregister.cpp sysdbusregister.h
)
target_link_libraries(lingmo-search-systemdbus PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)
install(FILES conf/com.lingmo.search.qt.systemdbus.service DESTINATION /usr/share/dbus-1/system-services/)
install(FILES conf/com.lingmo.search.qt.systemdbus.conf DESTINATION /etc/dbus-1/system.d/)

install(TARGETS lingmo-search-systemdbus RUNTIME DESTINATION /usr/bin)