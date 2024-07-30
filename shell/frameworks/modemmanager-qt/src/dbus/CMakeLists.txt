find_package(Qt4 REQUIRED)

add_definitions(${QT_DEFINITIONS})
set( INTERFACE_INTROSPECTION_XML_FILES
   introspection/org.freedesktop.ModemManager1.Call.xml
   introspection/org.freedesktop.ModemManager1.Bearer.xml
   introspection/org.freedesktop.ModemManager1.Modem.Firmware.xml
   introspection/org.freedesktop.ModemManager1.Modem.Location.xml
   introspection/org.freedesktop.ModemManager1.Modem.Messaging.xml
   introspection/org.freedesktop.ModemManager1.Modem.Modem3gpp.Ussd.xml
   introspection/org.freedesktop.ModemManager1.Modem.Modem3gpp.xml
   introspection/org.freedesktop.ModemManager1.Modem.ModemCdma.xml
   introspection/org.freedesktop.ModemManager1.Modem.Oma.xml
   introspection/org.freedesktop.ModemManager1.Modem.Signal.xml
   introspection/org.freedesktop.ModemManager1.Modem.Simple.xml
   introspection/org.freedesktop.ModemManager1.Modem.Time.xml
   introspection/org.freedesktop.ModemManager1.Modem.Voice.xml
   introspection/org.freedesktop.ModemManager1.Modem.xml
   introspection/org.freedesktop.ModemManager1.Sim.xml
   introspection/org.freedesktop.ModemManager1.Sms.xml
   introspection/org.freedesktop.ModemManager1.xml
#    introspection/wip-org.freedesktop.ModemManager1.Modem.Contacts.xml
   )

set( ADAPTER_INTROSPECTION_XML_FILES
   )

set_property( SOURCE ${INTERFACE_INTROSPECTION_XML_FILES} PROPERTY INCLUDE generictypes.h)
set_source_files_properties( ${INTERFACE_INTROSPECTION_XML_FILES} ${ADAPTER_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE )

set( UNUSED_XML
)


set( FILES_TO_BUILD
   )
set ( UNUSED_FILES
)

set( UNUSED_GENERATED_FILES
)

QT4_ADD_DBUS_INTERFACES( FILES_TO_BUILD ${INTERFACE_INTROSPECTION_XML_FILES} )

add_library(modemmanagerdbus ${FILES_TO_BUILD})

TARGET_LINK_LIBRARIES(modemmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})
