set(kio_obexftp_SRCS
    kioobexftp.cpp
    transferfilejob.cpp
)

set(kded_obexftp.xml ${CMAKE_SOURCE_DIR}/src/interfaces/kded_obexftp.xml)
qt_add_dbus_interface(kio_obexftp_SRCS ${kded_obexftp.xml} kdedobexftp)
ecm_qt_declare_logging_category(
    kio_obexftp_SRCS
    HEADER bluedevil_kio_obexftp.h
    IDENTIFIER BLUEDEVIL_KIO_OBEXFTP_LOG
    CATEGORY_NAME org.kde.lingmo.bluedevil.kioobexftp
    DESCRIPTION "BlueDevil (kio obexftp)"
    EXPORT BLUEDEVIL
)
add_library(kio_obexftp MODULE ${kio_obexftp_SRCS})

target_link_libraries(kio_obexftp
    Qt::Core
    Qt::DBus
    Qt::Network

    KF6::BluezQt
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
)

install(TARGETS kio_obexftp DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kio)
