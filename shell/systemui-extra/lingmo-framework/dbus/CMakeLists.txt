find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Core DBus)

add_subdirectory(devices)

aux_source_directory(SOURCES  devices/README.md)
