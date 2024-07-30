# Steps to generate the dbus interface files
# 1) Place the NetworkManager introspection dbus xml in introspection/
# 2) Update the list of xml files below that we want to generate interfaces for
# 3) Enable dbus subdirectory in ../CMakeLists.txt
#    Make, in order to generate the interface sources
# 3.1) Add annotations to the xml so qdbusxml2cpp knows what type or typedef to use
#      Add typedefs as needed to generictypes.h
# 4)   Add the list of generated files needed for the plugin to FILES_TO_BUILD
# 6) Copy the working generated interface sources into the source dir
# 5) Edit the sources as required to get them to build
# 5.1) Fix the inclusion guards as the compiler ignores anything after a '-' so the compiler thinks
#      headers have already been included
# Run do-hand-edits to update the diff of the interfaces vs what is generated; this speeds up regenerating them since you just have to apply the diff to make them compile

set(INTERFACE_INTROSPECTION_XML_FILES
    introspection/nm-agent-manager.xml
    introspection/org.freedesktop.NetworkManager.AccessPoint.xml
    introspection/org.freedesktop.NetworkManager.AgentManager.xml
    introspection/org.freedesktop.NetworkManager.Connection.Active.xml
    introspection/org.freedesktop.NetworkManager.Device.Adsl.xml
    introspection/org.freedesktop.NetworkManager.Device.Bluetooth.xml
    introspection/org.freedesktop.NetworkManager.Device.Bond.xml
    introspection/org.freedesktop.NetworkManager.Device.Bridge.xml
    introspection/org.freedesktop.NetworkManager.Device.Generic.xml
    introspection/org.freedesktop.NetworkManager.Device.Gre.xml
    introspection/org.freedesktop.NetworkManager.Device.Infiniband.xml
    introspection/org.freedesktop.NetworkManager.Device.IPTunnel.xml
    introspection/org.freedesktop.NetworkManager.Device.Macsec.xml
    introspection/org.freedesktop.NetworkManager.Device.Macvlan.xml
    introspection/org.freedesktop.NetworkManager.Device.Modem.xml
    introspection/org.freedesktop.NetworkManager.Device.OlpcMesh.xml
    introspection/org.freedesktop.NetworkManager.Device.Statistics.xml
    introspection/org.freedesktop.NetworkManager.Device.Team.xml
    introspection/org.freedesktop.NetworkManager.Device.Tun.xml
    introspection/org.freedesktop.NetworkManager.Device.Veth.xml
    introspection/org.freedesktop.NetworkManager.Device.Vlan.xml
    introspection/org.freedesktop.NetworkManager.Device.Vxlan.xml
    introspection/org.freedesktop.NetworkManager.Device.WiMax.xml
    introspection/org.freedesktop.NetworkManager.Device.Wired.xml
    introspection/org.freedesktop.NetworkManager.Device.Wireless.xml
    introspection/org.freedesktop.NetworkManager.Device.xml
    introspection/org.freedesktop.NetworkManager.DHCP4Config.xml
    introspection/org.freedesktop.NetworkManager.DHCP6Config.xml
    introspection/org.freedesktop.NetworkManager.DnsManager.xml
    introspection/org.freedesktop.NetworkManager.Checkpoint.xml
    introspection/org.freedesktop.NetworkManager.IP4Config.xml
    introspection/org.freedesktop.NetworkManager.IP6Config.xml
    introspection/org.freedesktop.NetworkManager.PPP.xml
#     introspection/org.freedesktop.NetworkManager.Settings.Connection.xml
    introspection/org.freedesktop.NetworkManager.Settings.xml
    introspection/org.freedesktop.NetworkManager.VPN.Connection.xml
    introspection/org.freedesktop.NetworkManager.VPN.Plugin.xml
    introspection/org.freedesktop.NetworkManager.WiMax.Nsp.xml
    introspection/org.freedesktop.NetworkManager.xml
   )

set_property(SOURCE ${INTERFACE_INTROSPECTION_XML_FILES} PROPERTY INCLUDE generictypes.h)

set(ADAPTER_INTROSPECTION_XML_FILES
   )

set_source_files_properties(${INTERFACE_INTROSPECTION_XML_FILES} ${ADAPTER_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE)

set(FILES_TO_BUILD
   )

qt_add_dbus_interfaces(FILES_TO_BUILD ${INTERFACE_INTROSPECTION_XML_FILES})
qt_add_dbus_adaptor(FILES_TO_BUILD introspection/org.freedesktop.NetworkManager.SecretAgent.xml "secretagent.h" "NetworkManager::SecretAgent")
foreach(adaptor ${ADAPTER_INTROSPECTION_XML_FILES})
    qt_add_dbus_adaptor(FILES_TO_BUILD ${adaptor} "generictypes.h")
endforeach(adaptor ${ADAPTER_INTROSPECTION_XML_FILES})

add_library(networkmanagerdbus ${FILES_TO_BUILD})

target_link_libraries(networkmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})
