set(bluezqt_SRCS
    a2dp-codecs.c
    manager.cpp
    manager_p.cpp
    adapter.cpp
    adapter_p.cpp
    battery.cpp
    device.cpp
    device_p.cpp
    gattapplication.cpp
    gattapplication_p.cpp
    gattcharacteristic.cpp
    gattcharacteristic_p.cpp
    gattcharacteristicadaptor.cpp
    gattdescriptor.cpp
    gattdescriptor_p.cpp
    gattdescriptoradaptor.cpp
    gattmanager.cpp
    gattmanager_p.cpp
    gattservice.cpp
    gattservice_p.cpp
    gattserviceadaptor.cpp
    gattserviceremote.cpp
    gattserviceremote_p.cpp
    gattcharacteristicremote.cpp
    gattcharacteristicremote_p.cpp
    gattdescriptorremote.cpp
    gattdescriptorremote_p.cpp
    input.cpp
    leadvertisement.cpp
    leadvertisement_p.cpp
    leadvertisementadaptor.cpp
    leadvertisingmanager.cpp
    media.cpp
    mediaendpoint.cpp
    mediaendpoint_p.cpp
    mediaendpointadaptor.cpp
    mediaplayer.cpp
    mediaplayer_p.cpp
    mediaplayertrack.cpp
    mediatransport.cpp
    mediatransport_p.cpp
    objectmanageradaptor.cpp
    devicesmodel.cpp
    job.cpp
    initmanagerjob.cpp
    initobexmanagerjob.cpp
    utils.cpp
    agent.cpp
    agentadaptor.cpp
    profile.cpp
    profileadaptor.cpp
    pendingcall.cpp
    request.cpp
    rfkill.cpp
    obexmanager.cpp
    obexmanager_p.cpp
    obexagent.cpp
    obexagentadaptor.cpp
    obextransfer.cpp
    obexsession.cpp
    obexobjectpush.cpp
    obexfiletransfer.cpp
    obexfiletransferentry.cpp
)

ecm_qt_declare_logging_category(bluezqt_SRCS
    HEADER debug.h
    IDENTIFIER BLUEZQT
    CATEGORY_NAME kf.bluezqt
    OLD_CATEGORY_NAMES org.kde.bluez
    DESCRIPTION "BluezQt"
    EXPORT BLUEZQT
)

set(dbusobjectmanager_xml ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.freedesktop.DBus.ObjectManager.xml)
set_source_files_properties(${dbusobjectmanager_xml} PROPERTIES INCLUDE "bluezqt_dbustypes.h")
qt_add_dbus_interface(bluezqt_SRCS ${dbusobjectmanager_xml} dbusobjectmanager)

set(obexfiletransfer1_xml ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.obex.FileTransfer1.xml)
set_source_files_properties(${obexfiletransfer1_xml} PROPERTIES INCLUDE "bluezqt_dbustypes.h")
qt_add_dbus_interface(bluezqt_SRCS ${obexfiletransfer1_xml} obexfiletransfer1)

qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattDescriptor1.xml bluezgattdescriptor1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattService1.xml bluezgattservice1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.freedesktop.DBus.Properties.xml dbusproperties)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Adapter1.xml bluezadapter1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.AgentManager1.xml bluezagentmanager1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Battery1.xml bluezbattery1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattCharacteristic1.xml bluezgattcharacteristic1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattManager1.xml bluezgattmanager1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.ProfileManager1.xml bluezprofilemanager1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Device1.xml bluezdevice1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.LEAdvertisement1.xml bluezleadvertisement1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.LEAdvertisingManager1.xml bluezleadvertisingmanager1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Media1.xml bluezmedia1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.MediaEndpoint1.xml bluezmediaendpoint1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.MediaPlayer1.xml bluezmediaplayer1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.MediaTransport1.xml bluezmediatransport1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.obex.AgentManager1.xml obexagentmanager1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.obex.Client1.xml obexclient1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.obex.Transfer1.xml obextransfer1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.obex.Session1.xml obexsession1)
qt_add_dbus_interface(bluezqt_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.obex.ObjectPush1.xml obexobjectpush1)

add_library(KF6BluezQt ${bluezqt_SRCS})
add_library(KF6::BluezQt ALIAS KF6BluezQt)

qt_extract_metatypes(KF6BluezQt)

ecm_generate_export_header(KF6BluezQt
    BASE_NAME BluezQt
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6BluezQt INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/BluezQt>")

target_link_libraries(KF6BluezQt
    PUBLIC
        Qt6::Core
    PRIVATE
        Qt6::DBus
        Qt6::Network
)

set_target_properties(KF6BluezQt
    PROPERTIES VERSION ${BLUEZQT_VERSION}
    SOVERSION ${BLUEZQT_SOVERSION}
    EXPORT_NAME "BluezQt"
)

ecm_generate_headers(BluezQt_CamelCase_HEADERS
    HEADER_NAMES
        Types
        Manager
        Adapter
        Device
        GattApplication
        GattCharacteristic
        GattDescriptor
        GattManager
        GattService
        GattServiceRemote
        GattCharacteristicRemote
        GattDescriptorRemote
        Input
        LEAdvertisement
        LEAdvertisingManager
        Media
        MediaEndpoint
        MediaPlayer
        MediaPlayerTrack
        MediaTransport
        MediaTypes
        TPendingCall
        DevicesModel
        Job
        InitManagerJob
        InitObexManagerJob
        Services
        Agent
        Profile
        PendingCall
        Request
        Rfkill
        ObexManager
        ObexAgent
        ObexTransfer
        ObexSession
        ObexObjectPush
        ObexFileTransfer
        ObexFileTransferEntry

    REQUIRED_HEADERS BluezQt_HEADERS
    PREFIX BluezQt
)

# Install files
install(TARGETS KF6BluezQt
        EXPORT KF6BluezQtTargets
        ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${BluezQt_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/BluezQt/BluezQt
    COMPONENT Devel)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/bluezqt_export.h
    ${BluezQt_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/BluezQt/bluezqt
    COMPONENT Devel)

ecm_qt_install_logging_categories(
    EXPORT BLUEZQT
    FILE bluezqt.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)


if(BUILD_QCH)
    ecm_add_qch(
        KF6BluezQt_QCH
        NAME BluezQt
        BASE_NAME KF6BluezQt
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${BluezQt_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            BLUEZQT_EXPORT
            BLUEZQT_DEPRECATED
            BLUEZQT_DEPRECATED_EXPORT
            "BLUEZQT_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

