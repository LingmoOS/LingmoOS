# add_subdirectory(dbus)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/dbus
   ${CMAKE_CURRENT_SOURCE_DIR}/settings
)

set(NetworkManagerQt_PART_SRCS
    device.cpp
    accesspoint.cpp
    activeconnection.cpp
    adsldevice.cpp
    bluetoothdevice.cpp
    bonddevice.cpp
    bridgedevice.cpp
    connection.cpp
    dhcp4config.cpp
    dhcp6config.cpp
    devicestatistics.cpp
    dnsconfiguration.cpp
    dnsdomain.cpp
    infinibanddevice.cpp
    ipaddress.cpp
    iproute.cpp
    ipconfig.cpp
    manager.cpp
    modemdevice.cpp
    olpcmeshdevice.cpp
    secretagent.cpp
    settings.cpp
    utils.cpp
    vlandevice.cpp
    vpnconnection.cpp
    vpnplugin.cpp
    wimaxdevice.cpp
    wimaxnsp.cpp
    wireddevice.cpp
    wirelessdevice.cpp
    wirelessnetwork.cpp
    generictypes.cpp
    genericdevice.cpp
    gredevice.cpp
    iptunneldevice.cpp
    macvlandevice.cpp
    teamdevice.cpp
    tundevice.cpp
    vethdevice.cpp
    wimaxdevice.cpp
    wimaxnsp.cpp
    wireguarddevice.cpp
)

set(NetworkManagerQt_SETTINGS_SRCS
    settings/adslsetting.cpp
    settings/bluetoothsetting.cpp
    settings/bondsetting.cpp
    settings/bridgesetting.cpp
    settings/bridgeportsetting.cpp
    settings/cdmasetting.cpp
    settings/connectionsettings.cpp
    settings/dcbsetting.cpp
    settings/gsmsetting.cpp
    settings/iptunnelsetting.cpp
    settings/ipv4setting.cpp
    settings/ipv6setting.cpp
    settings/infinibandsetting.cpp
    settings/macsecsetting.cpp
    settings/matchsetting.cpp
    settings/olpcmeshsetting.cpp
    settings/ovsbridgesetting.cpp
    settings/ovsinterfacesetting.cpp
    settings/ovspatchsetting.cpp
    settings/ovsportsetting.cpp
    settings/pppsetting.cpp
    settings/pppoesetting.cpp
    settings/proxysetting.cpp
    settings/setting.cpp
    settings/serialsetting.cpp
    settings/security8021xsetting.cpp
    settings/teamportsetting.cpp
    #settings/template.cpp
    settings/vlansetting.cpp
    settings/vpnsetting.cpp
    settings/wimaxsetting.cpp
    settings/wiredsetting.cpp
    settings/wirelesssetting.cpp
    settings/wirelesssecuritysetting.cpp
    settings/teamsetting.cpp
    settings/tcsetting.cpp
    settings/genericsetting.cpp
    settings/tunsetting.cpp
    settings/usersetting.cpp
    settings/vxlansetting.cpp
    settings/wireguardsetting.cpp
)

set(DBUS_INTERFACE_SRCS
    dbus/accesspointinterface.cpp
    dbus/activeconnectioninterface.cpp
    dbus/adsldeviceinterface.cpp
    dbus/agentmanagerinterface.cpp
    dbus/bluetoothdeviceinterface.cpp
    dbus/bonddeviceinterface.cpp
    dbus/bridgedeviceinterface.cpp
    dbus/connectioninterface.cpp
    dbus/deviceinterface.cpp
    dbus/devicestatisticsinterface.cpp
    dbus/dhcp4configinterface.cpp
    dbus/dhcp6configinterface.cpp
    dbus/dnsmanagerinterface.cpp
    dbus/genericdeviceinterface.cpp
    dbus/gredeviceinterface.cpp
    dbus/checkpointinterface.cpp
    dbus/infinibanddeviceinterface.cpp
    dbus/iptunneldeviceinterface.cpp
    dbus/ip4configinterface.cpp
    dbus/ip6configinterface.cpp
    dbus/macsecdeviceinterface.cpp
    dbus/macvlandeviceinterface.cpp
    dbus/modemdeviceinterface.cpp
    dbus/networkmanagerinterface.cpp
    dbus/olpcmeshdeviceinterface.cpp
    dbus/pppinterface.cpp
    dbus/secretagentadaptor.cpp
    dbus/settingsinterface.cpp
    dbus/teamdeviceinterface.cpp
    dbus/tundeviceinterface.cpp
    dbus/vethdeviceinterface.cpp
    dbus/vlandeviceinterface.cpp
    dbus/vpnconnectioninterface.cpp
    dbus/vpnplugininterface.cpp
    dbus/vxlandeviceinterface.cpp
    dbus/wimaxdeviceinterface.cpp
    dbus/wimaxnspinterface.cpp
    dbus/wireddeviceinterface.cpp
    dbus/wirelessdeviceinterface.cpp
    dbus/wireguarddeviceinterface.cpp
)

ecm_qt_declare_logging_category(KF6NetworkManagerQt
    HEADER nmdebug.h
    IDENTIFIER NMQT
    CATEGORY_NAME kf.networkmanagerqt
    OLD_CATEGORY_NAMES networkmanager-qt
    DEFAULT_SEVERITY Warning
    DESCRIPTION "NetworkManagerQt"
    EXPORT NETWORMANAGERQT
)

add_library(KF6NetworkManagerQt SHARED ${NetworkManagerQt_PART_SRCS} ${NetworkManagerQt_SETTINGS_SRCS} ${DBUS_INTERFACE_SRCS})
add_library(KF6::NetworkManagerQt ALIAS KF6NetworkManagerQt)

qt_extract_metatypes(KF6NetworkManagerQt)

ecm_generate_export_header(KF6NetworkManagerQt
    EXPORT_FILE_NAME ${NetworkManagerQt_BINARY_DIR}/networkmanagerqt/networkmanagerqt_export.h
    BASE_NAME NetworkManagerQt
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
)

target_include_directories(KF6NetworkManagerQt INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/NetworkManagerQt>")
# for examples to build
target_include_directories(KF6NetworkManagerQt PUBLIC "$<BUILD_INTERFACE:${NetworkManagerQt_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}/settings;${CMAKE_CURRENT_BINARY_DIR}>")

target_link_libraries(KF6NetworkManagerQt PUBLIC Qt6::Core Qt6::Network Qt6::DBus PkgConfig::NetworkManager PkgConfig::Gio)

set_target_properties(KF6NetworkManagerQt PROPERTIES VERSION ${NETWORKMANAGERQT_VERSION}
                                                   SOVERSION ${NETWORKMANAGERQT_SOVERSION}
                                                   EXPORT_NAME NetworkManagerQt
)

########### static lib for tests  ###############
add_library(KF6NetworkManagerQt_static STATIC ${NetworkManagerQt_PART_SRCS} ${NetworkManagerQt_SETTINGS_SRCS} ${DBUS_INTERFACE_SRCS})
set_target_properties(KF6NetworkManagerQt_static PROPERTIES COMPILE_FLAGS -DNMQT_STATIC=1)

target_link_libraries(KF6NetworkManagerQt_static PUBLIC Qt6::Core Qt6::Network Qt6::DBus PkgConfig::NetworkManager PkgConfig::Gio)
target_include_directories(KF6NetworkManagerQt_static PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")

ecm_generate_headers(NetworkManagerQt_CamelCase_HEADERS
  HEADER_NAMES
  AccessPoint
  ActiveConnection
  AdslDevice
  BluetoothDevice
  BondDevice
  BridgeDevice
  Connection
  Device
  DeviceStatistics
  Dhcp4Config
  Dhcp6Config
  DnsConfiguration
  DnsDomain
  GenericDevice
  GenericTypes
  GreDevice
  InfinibandDevice
  IpAddress
  IpConfig
  IpRoute
  IpTunnelDevice
  MacVlanDevice
  Manager
  ModemDevice
  OlpcMeshDevice
  SecretAgent
  Settings
  TeamDevice
  TunDevice
  Utils
  VethDevice
  VlanDevice
  VpnConnection
  VpnPlugin
  WimaxDevice
  WimaxNsp
  WiredDevice
  WirelessDevice
  WirelessNetwork
  WireGuardDevice

  REQUIRED_HEADERS NetworkManagerQt_HEADERS
  PREFIX NetworkManagerQt
)

ecm_generate_headers(NetworkManagerQt_SETTINGS_CamelCase_HEADERS
  HEADER_NAMES
  AdslSetting
  BluetoothSetting
  BondSetting
  BridgeSetting
  BridgePortSetting
  CdmaSetting
  ConnectionSettings
  GsmSetting
  GenericSetting
  InfinibandSetting
  IpTunnelSetting
  Ipv4Setting
  Ipv6Setting
  OlpcMeshSetting
  PppoeSetting
  PppSetting
  Security8021xSetting
  SerialSetting
  Setting
  TeamSetting
  TunSetting
  VlanSetting
  VpnSetting
  WimaxSetting
  WiredSetting
  WireguardSetting
  WirelessSecuritySetting
  WirelessSetting

  RELATIVE settings
  REQUIRED_HEADERS NetworkManagerQt_SETTINGS_HEADERS
  PREFIX NetworkManagerQt
)

ecm_add_qml_module(networkmanagerqtqml URI org.kde.networkmanager GENERATE_PLUGIN_SOURCE)
target_sources(networkmanagerqtqml PRIVATE qmltypes.h)
target_link_libraries(networkmanagerqtqml PRIVATE Qt6::Qml KF6NetworkManagerQt)
ecm_finalize_qml_module(networkmanagerqtqml)

install(TARGETS KF6NetworkManagerQt EXPORT KF6NetworkManagerQtTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${NetworkManagerQt_CamelCase_HEADERS}
  ${NetworkManagerQt_SETTINGS_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/NetworkManagerQt/NetworkManagerQt COMPONENT Devel
)

install(FILES
  ${NetworkManagerQt_BINARY_DIR}/networkmanagerqt/networkmanagerqt_export.h
  ${NetworkManagerQt_HEADERS}
  ${NetworkManagerQt_SETTINGS_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/NetworkManagerQt/networkmanagerqt COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT NETWORMANAGERQT
    FILE networkmanagerqt.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6NetworkManagerQt_QCH
        NAME NetworkManagerQt
        BASE_NAME KF6NetworkManagerQt
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${NetworkManagerQt_HEADERS}
            ${NetworkManagerQt_SETTINGS_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Network_QCH
            Qt6DBus_QCH
        BLANK_MACROS
            NETWORKMANAGERQT_EXPORT
            NETWORKMANAGERQT_DEPRECATED
            NETWORKMANAGERQT_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
