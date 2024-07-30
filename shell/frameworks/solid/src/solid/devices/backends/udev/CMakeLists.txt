set(backend_sources
    udevdevice.cpp
    udevmanager.cpp
    udevdeviceinterface.cpp
    udevgenericinterface.cpp
    cpuinfo.cpp
    cpuinfo_arm.cpp
    udevprocessor.cpp
    udevcamera.cpp
    udevportablemediaplayer.cpp
    udevblock.cpp
    ../shared/udevqtclient.cpp
    ../shared/udevqtdevice.cpp
)
set(backend_libs UDev::UDev)

set(UDEV_DETAILED_OUTPUT OFF CACHE BOOL "provide extended output regarding udev events")
if(UDEV_DETAILED_OUTPUT)
    add_definitions(-DUDEV_DETAILED_OUTPUT)
endif()

# check for media-player-info (runtime-only optional dependency)
find_package(MediaPlayerInfo)
set_package_properties(MediaPlayerInfo PROPERTIES
    DESCRIPTION "Enables identification and querying of portable media players"
    PURPOSE "Runtime-only dependency of the udev solid backend. Support for m-p-i is included even if not found during build"
    URL "http://www.freedesktop.org/wiki/Software/media-player-info"
    TYPE RUNTIME
)
