set(backend_sources
    fstabmanager.cpp
    fstabdevice.cpp
    fstabnetworkshare.cpp
    fstabstorageaccess.cpp
    fstabhandling.cpp
    fstabwatcher.cpp
)

ecm_qt_declare_logging_category(backend_sources
    HEADER fstab_debug.h
    IDENTIFIER Solid::Backends::Fstab::FSTAB_LOG
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME kf.solid.backends.fstab
    OLD_CATEGORY_NAMES org.kde.solid.fstab
    DESCRIPTION "Fstab (Solid)"
    EXPORT SOLID
)
