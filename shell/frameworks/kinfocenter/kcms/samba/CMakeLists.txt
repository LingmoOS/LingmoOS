# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_samba\")

set(kcm_samba_PART_SRCS
    smbmountmodel.cpp
    ksambasharemodel.cpp
    smbmountmodel.h
    ksambasharemodel.h
)

qt_add_dbus_interface(kcm_samba_PART_SRCS org.freedesktop.Avahi.Server.xml org.freedesktop.Avahi.Server)

# Intermediate lib for use in testing.
add_library(kcm_samba_static STATIC ${kcm_samba_PART_SRCS})
set_property(TARGET kcm_samba_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kcm_samba_static
    KF6::KIOCore
    KF6::Solid
    KF6::I18n
    KF6::KCMUtils
    KF6::KIOWidgets
)

set(kcm_samba_SRCS main.cpp)
qt_add_dbus_interface(kcm_samba_SRCS org.freedesktop.DBus.Properties.xml org.freedesktop.DBus.Properties)
kinfocenter_add_kcm(kcm_samba SOURCES ${kcm_samba_SRCS} INSTALL_NAMESPACE "lingmo/kcms/kinfocenter")
target_link_libraries(kcm_samba PRIVATE
    KF6::KCMUtilsQuick
    kcm_samba_static
)

add_subdirectory(autotests)
