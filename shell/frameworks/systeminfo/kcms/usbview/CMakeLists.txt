# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmusb\")

set(kcm_usb_PART_SRCS kcmusb.cpp
    kcmusb.h
    usbdb.cpp
    usbdb.h
    usbdevices.cpp
    usbdevices.h )

kcoreaddons_add_plugin(kcm_usb SOURCES ${kcm_usb_PART_SRCS} INSTALL_NAMESPACE "lingmo/kcms/kinfocenter")

target_link_libraries(kcm_usb KF6::KCMUtils KF6::I18n Libusb-1.0)
