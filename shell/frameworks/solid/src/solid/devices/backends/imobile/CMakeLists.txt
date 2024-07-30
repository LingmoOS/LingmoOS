set(backend_sources
    imobile.cpp
    imobiledevice.cpp
    imobilemanager.cpp
    imobiledeviceinterface.cpp
    imobileportablemediaplayer.cpp
)
set(backend_libs IMobileDevice::IMobileDevice PList::PList)

ecm_qt_declare_logging_category(backend_sources
    HEADER imobile_debug.h
    IDENTIFIER Solid::Backends::IMobile::IMOBILE
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME kf.solid.backends.imobile
    DESCRIPTION "IMobileDevice (Solid)"
    EXPORT SOLID
)
