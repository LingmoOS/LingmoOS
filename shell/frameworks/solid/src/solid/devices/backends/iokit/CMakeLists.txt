set(backend_sources
    iokitmanager.cpp
    iokitdevice.cpp
    cfhelper.cpp
    dadictionary.cpp
    iokitdeviceinterface.cpp
    iokitgenericinterface.cpp
    iokitprocessor.cpp
    iokitbattery.cpp
    iokitblock.cpp
    iokitstorage.cpp
    iokitvolume.cpp
    iokitstorageaccess.cpp
    iokitopticaldrive.cpp
    iokitopticaldisc.cpp
)
set(backend_libs ${IOKIT_LIBRARY} "-framework DiskArbitration")
