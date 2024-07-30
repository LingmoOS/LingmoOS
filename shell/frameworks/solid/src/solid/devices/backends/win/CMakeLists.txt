set(backend_sources
    winblock.cpp
    winopticaldrive.cpp
    wininterface.cpp
    wingenericinterface.cpp
    windevice.cpp
    windevicemanager.cpp
    winopticaldisc.cpp
    winprocessor.cpp
    #wmiportablemediaplayer.cpp
    winstorageaccess.cpp
    winstoragedrive.cpp
    winstoragevolume.cpp
    winbattery.cpp
)
set(backend_libs Qt6::Network setupapi)
