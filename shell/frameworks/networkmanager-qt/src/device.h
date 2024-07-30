/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DEVICE_H
#define NETWORKMANAGERQT_DEVICE_H

#include <QObject>
#include <QSharedPointer>

#include "activeconnection.h"
#include "devicestatistics.h"
#include "dhcp4config.h"
#include "dhcp6config.h"
#include "generictypes.h"
#include "ipconfig.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class DevicePrivate;
class DeviceStateReason;
class DeviceStateReasonPrivate;

/**
 * This class represents a common device interface
 */
class NETWORKMANAGERQT_EXPORT Device : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString uni READ uni)
    Q_PROPERTY(QString interfaceName READ interfaceName)
    Q_PROPERTY(QString ipInterfaceName READ ipInterfaceName)
    Q_PROPERTY(QString driver READ driver)
    Q_PROPERTY(QString driverVersion READ driverVersion)
    Q_PROPERTY(QString firmwareVersion READ firmwareVersion)
    Q_PROPERTY(QVariant genericCapabilities READ capabilitiesV)
    Q_PROPERTY(QHostAddress ipV4Address READ ipV4Address)
    Q_PROPERTY(bool managed READ managed)
    Q_PROPERTY(uint mtu READ mtu)
    Q_PROPERTY(Interfaceflags InterfaceFlags READ interfaceFlags)
    Q_PROPERTY(bool nmPluginMissing READ nmPluginMissing)
    Q_PROPERTY(MeteredStatus metered READ metered)
    Q_PROPERTY(QString udi READ udi)
    Q_PROPERTY(bool firmwareMissing READ firmwareMissing)
    Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect)
    Q_PROPERTY(DeviceStateReason stateReason READ stateReason)
    Q_PROPERTY(State state READ state)
    Q_PROPERTY(NetworkManager::DeviceStatistics::Ptr deviceStatistics READ deviceStatistics)

public:
    typedef QSharedPointer<Device> Ptr;
    typedef QList<Ptr> List;
    /**
     * Device connection states describe the possible states of a
     * network connection from the user's point of view.  For
     * simplicity, states from several different layers are present -
     * this is a high level view
     */
    enum State {
        UnknownState = 0, /**< The device is in an unknown state */
        Unmanaged = 10, /**< The device is recognized but not managed by NetworkManager */
        Unavailable = 20, /**< The device cannot be used (carrier off, rfkill, etc) */
        Disconnected = 30, /**< The device is not connected */
        Preparing = 40, /**< The device is preparing to connect */
        ConfiguringHardware = 50, /**< The device is being configured */
        NeedAuth = 60, /**< The device is awaiting secrets necessary to continue connection */
        ConfiguringIp = 70, /**< The IP settings of the device are being requested and configured */
        CheckingIp = 80, /**< The device's IP connectivity ability is being determined */
        WaitingForSecondaries = 90, /**< The device is waiting for secondary connections to be activated */
        Activated = 100, /**< The device is active */
        Deactivating = 110, /**< The device's network connection is being torn down */
        Failed = 120, /**< The device is in a failure state following an attempt to activate it */
    };
    Q_ENUM(State)

    /**
     * Enums describing the reason for a connection state change
     * @note StateChangeReasons NewActivation, ParentChanged, ParentManagedChanged are available in runtime NM >= 1.0.4
     */
    enum StateChangeReason {
        UnknownReason = 0,
        NoReason = 1,
        NowManagedReason = 2,
        NowUnmanagedReason = 3,
        ConfigFailedReason = 4,
        ConfigUnavailableReason = 5,
        ConfigExpiredReason = 6,
        NoSecretsReason = 7,
        AuthSupplicantDisconnectReason = 8,
        AuthSupplicantConfigFailedReason = 9,
        AuthSupplicantFailedReason = 10,
        AuthSupplicantTimeoutReason = 11,
        PppStartFailedReason = 12,
        PppDisconnectReason = 13,
        PppFailedReason = 14,
        DhcpStartFailedReason = 15,
        DhcpErrorReason = 16,
        DhcpFailedReason = 17,
        SharedStartFailedReason = 18,
        SharedFailedReason = 19,
        AutoIpStartFailedReason = 20,
        AutoIpErrorReason = 21,
        AutoIpFailedReason = 22,
        ModemBusyReason = 23,
        ModemNoDialToneReason = 24,
        ModemNoCarrierReason = 25,
        ModemDialTimeoutReason = 26,
        ModemDialFailedReason = 27,
        ModemInitFailedReason = 28,
        GsmApnSelectFailedReason = 29,
        GsmNotSearchingReason = 30,
        GsmRegistrationDeniedReason = 31,
        GsmRegistrationTimeoutReason = 32,
        GsmRegistrationFailedReason = 33,
        GsmPinCheckFailedReason = 34,
        FirmwareMissingReason = 35,
        DeviceRemovedReason = 36,
        SleepingReason = 37,
        ConnectionRemovedReason = 38,
        UserRequestedReason = 39,
        CarrierReason = 40,
        ConnectionAssumedReason = 41,
        SupplicantAvailableReason = 42,
        ModemNotFoundReason = 43,
        BluetoothFailedReason = 44,
        GsmSimNotInserted = 45,
        GsmSimPinRequired = 46,
        GsmSimPukRequired = 47,
        GsmSimWrong = 48,
        InfiniBandMode = 49,
        DependencyFailed = 50,
        Br2684Failed = 51,
        ModemManagerUnavailable = 52,
        SsidNotFound = 53,
        SecondaryConnectionFailed = 54,
        DcbFcoeFailed = 55,
        TeamdControlFailed = 56,
        ModemFailed = 57,
        ModemAvailable = 58,
        SimPinIncorrect = 59,
        NewActivation = 60,
        ParentChanged = 61,
        ParentManagedChanged = 62,
        Reserved = 65536,
    };
    Q_ENUM(StateChangeReason)

    enum MeteredStatus {
        UnknownStatus = 0, /**< The device metered status is unknown. */
        Yes = 1, /**< The device is metered and the value was statically set. */
        No = 2, /**< The device is not metered and the value was statically set. */
        GuessYes = 3, /**< The device is metered and the value was guessed. */
        GuessNo = 4, /**< The device is not metered and the value was guessed. */
    };
    Q_ENUM(MeteredStatus)

    /**
     * Possible device capabilities
     */
    enum Capability {
        IsManageable = 0x1, /**< denotes that the device can be controlled by this API */
        SupportsCarrierDetect = 0x2, /**< the device informs us when it is plugged in to the medium */
    };
    Q_ENUM(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    /**
     * Possible device interfaceflags
     */
    enum Interfaceflag {
        None = NM_DEVICE_INTERFACE_FLAG_NONE,                   /**< no flags set */
        Up = NM_DEVICE_INTERFACE_FLAG_UP,                       /**< Corresponds to kernel IFF_UP */
        LowerUp = NM_DEVICE_INTERFACE_FLAG_LOWER_UP,            /**< Corresponds to kernel IFF_LOWER_UP */
        Carrier = NM_DEVICE_INTERFACE_FLAG_CARRIER              /**< the interface has carrier */
    };
    Q_ENUM(Interfaceflag)
    Q_DECLARE_FLAGS(Interfaceflags, Interfaceflag)
    Q_FLAG(Interfaceflags)

    /**
     * Device type
     */
    enum Type {
        UnknownType = NM_DEVICE_TYPE_UNKNOWN, /**< Unknown device type */
        Ethernet = NM_DEVICE_TYPE_ETHERNET, /**< Ieee8023 wired ethernet */
        Wifi = NM_DEVICE_TYPE_WIFI, /**< the Ieee80211 family of wireless networks */
        Unused1 = NM_DEVICE_TYPE_UNUSED1, /**< Currently unused */
        Unused2 = NM_DEVICE_TYPE_UNUSED2, /**< Currently unused */
        Bluetooth = NM_DEVICE_TYPE_BT, /**< network bluetooth device (usually a cell phone) */
        OlpcMesh = NM_DEVICE_TYPE_OLPC_MESH, /**< OLPC Mesh networking device */
        Wimax = NM_DEVICE_TYPE_WIMAX, /**< WiMax WWAN technology */
        Modem = NM_DEVICE_TYPE_MODEM, /**< POTS, GSM, CDMA or LTE modems */
        InfiniBand = NM_DEVICE_TYPE_INFINIBAND, /**< Infiniband network device */
        Bond = NM_DEVICE_TYPE_BOND, /**< Bond virtual device */
        Vlan = NM_DEVICE_TYPE_VLAN, /**< Vlan virtual device */
        Adsl = NM_DEVICE_TYPE_ADSL, /**< ADSL modem device */
        Bridge = NM_DEVICE_TYPE_BRIDGE, /**< Bridge virtual device */
        Generic = NM_DEVICE_TYPE_GENERIC, /**< Generic device @since 1.0.0 */
        Team = NM_DEVICE_TYPE_TEAM, /**< Team master device @since 1.0.0 */
        Gre, /**< Gre virtual device @since 1.2.0, @deprecated use IpTunnel instead*/
        MacVlan, /**< MacVlan virtual device @since 1.2.0 */
        Tun, /**< Tun virtual device @since 1.2.0 */
        Veth, /**< Veth virtual device @since 1.2.0 */
        IpTunnel, /**< IP Tunneling Device @since 1.2.0 */
        VxLan, /**< Vxlan Device @since 1.2.0 */
        MacSec, /**< MacSec Device @since 1.6.0 */
        Dummy, /**< Dummy Device @since 1.8.0 */
        Ppp, /**< Ppp Device @since 1.10 */
        OvsInterface, /**< OvsInterface Device @since 1.10 */
        OvsPort, /**< OvsPort Device @since 1.10 */
        OvsBridge, /**< OvsBridge Device @since 1.10 */
        Wpan, /**< Wpan Device @since 1.14 */
        Lowpan, /**< Lowpan Device @since 1.14 */
        WireGuard, /**< WireGuard Device @since 1.14 */
        WifiP2P, /**< WifiP2P Device @since 1.16 */
        VRF, /**< VRF (Virtual Routing and Forwarding) Device @since: 1.24 */
        Loopback, /**< Loopback Device @since 1.42 */
    };
    Q_ENUM(Type)
    Q_DECLARE_FLAGS(Types, Type)
    Q_FLAG(Types)

    /**
     * Creates a new device object.
     *
     * @param path UNI of the device
     */
    explicit Device(const QString &path, QObject *parent = nullptr);
    /**
     * Destroys a device object.
     */
    ~Device() override;
    /**
     * Retrieves the interface type.  This is a virtual function that will return the
     * proper type of all sub-classes.
     *
     * @returns the NetworkManager::Device::Type that corresponds to this device.
     */
    virtual Type type() const;
    /**
     * Retrieves the Unique Network Identifier (UNI) of the device.
     * This identifier is unique for each network and network interface in the system.
     *
     * @returns the Unique Network Identifier of the current device
     */
    QString uni() const;
    /**
     * The current active connection for this device
     *
     * @returns A valid ActiveConnection object or NULL if no active connection was found
     */
    NetworkManager::ActiveConnection::Ptr activeConnection() const;
    /**
     * Returns available connections for this device
     *
     * @returns List of availables connection
     */
    Connection::List availableConnections();
    /**
     * The system name for the network device
     */
    QString interfaceName() const;
    /**
     * The name of the device's data interface when available. This property
     * may not refer to the actual data interface until the device has
     * successfully established a data connection, indicated by the device's
     * state() becoming ACTIVATED.
     */
    QString ipInterfaceName() const;
    /**
     * Handle for the system driver controlling this network interface
     */
    QString driver() const;
    /**
     * The driver version.
     */
    QString driverVersion() const;
    /**
     * The firmware version.
     */
    QString firmwareVersion() const;
    /**
     * Reapplies connection settings on the interface.
     */
    QDBusPendingReply<> reapplyConnection(const NMVariantMapMap &connection, qulonglong version_id, uint flags);
    /**
     * Disconnects a device and prevents the device from automatically
     * activating further connections without user intervention.
     */
    QDBusPendingReply<> disconnectInterface();
    /**
     * Deletes a software device from NetworkManager and removes the interface from the system.
     * The method returns an error when called for a hardware device.
     *
     * @since 5.8.0
     *
     */
    QDBusPendingReply<> deleteInterface();
    /**
     * returns the current IPv4 address without the prefix
     * \sa ipV4Config()
     * \sa ipV6Config()
     * @deprecated
     */
    QHostAddress ipV4Address() const;
    /**
     * Get the current IPv4 configuration of this device.
     * Only valid when device is Activated.
     */
    IpConfig ipV4Config() const;
    /**
     * Get the current IPv6 configuration of this device.
     * Only valid when device is Activated.
     */
    IpConfig ipV6Config() const;

    /**
     * Get the DHCP options returned by the DHCP server
     * or a null pointer if the device is not Activated or does not
     * use DHCP configuration.
     */
    Dhcp4Config::Ptr dhcp4Config() const;

    /**
     * Get the DHCP options returned by the DHCP server
     * or a null pointer if the device is not Activated or does not
     * use DHCP configuration.
     */
    Dhcp6Config::Ptr dhcp6Config() const;

    /**
     * Retrieves the activation status of this network interface.
     *
     * @return true if this network interface is active, false otherwise
     */
    bool isActive() const;

    /**
     * Retrieves the device is valid.
     *
     * @return true if this device interface is valid, false otherwise
     */
    bool isValid() const;

    /**
     * Retrieves the current state of the device.
     * This is a high level view of the device. It is user oriented, so
     * actually it provides state coming from different layers.
     *
     * @return the current connection state
     * @see Device::State
     */
    State state() const;
    /**
     * Retrieves the maximum speed as reported by the device.
     * Note that this is only a design related piece of information, and that
     * the device might not reach this maximum.
     *
     * @return the device's maximum speed
     */
    int designSpeed() const;
    /**
     * Retrieves the capabilities supported by this device.
     *
     * @return the capabilities of the device
     */
    Capabilities capabilities() const;
    QVariant capabilitiesV() const;
    /**
     * Is the device currently being managed by NetworkManager?
     */
    bool managed() const;
    /**
      * The up or down flag for the device
      */
    Interfaceflags interfaceFlags() const;
    /**
     * Is the firmware needed by the device missing?
     */
    bool firmwareMissing() const;
    /**
     * If the device is allowed to autoconnect.
     */
    bool autoconnect() const;
    /**
     * The current state and reason for changing to that state.
     */
    DeviceStateReason stateReason() const;
    /**
     * Retrieves the Unique Device Identifier (UDI) of the device.
     * This identifier is unique for each device in the system.
     */
    QString udi() const;

    /**
     * @return If non-empty, an (opaque) indicator of the physical network
     * port associated with the device. This can be used to recognize
     * when two seemingly-separate hardware devices are actually just
     * different virtual interfaces to the same physical port.
     *
     * @since 0.9.9.0
     */
    QString physicalPortId() const;
    /**
     * The device MTU (maximum transmission unit)
     * @since 0.9.9.0
     *
     */
    uint mtu() const;

    /**
     * @return If TRUE, indicates the NetworkManager plugin for the device is likely
     * missing or misconfigured.
     * @since 5.14.0
     */
    bool nmPluginMissing() const;

    /**
     * @return Whether the amount of traffic flowing through the device is
     * subject to limitations, for example set by service providers.
     * @since 5.14.0
     */
    MeteredStatus metered() const;

    /**
     * If true, indicates the device is allowed to autoconnect.
     * If false, manual intervention is required before the device
     * will automatically connect to a known network, such as activating
     * a connection using the device, or setting this property to @p true.
     */
    void setAutoconnect(bool autoconnect);

    /**
     * Returns Device Statistics interface
     */
    DeviceStatistics::Ptr deviceStatistics() const;

    /**
     * Retrieves a specialized interface to interact with the device corresponding
     * to a given device interface.
     *
     * @returns a pointer to the device interface if it exists, @p 0 otherwise
     */
    template<class DevIface>
    DevIface *as()
    {
        return qobject_cast<DevIface *>(this);
    }

    /**
     * Retrieves a specialized interface to interact with the device corresponding
     * to a given device interface.
     *
     * @returns a pointer to the device interface if it exists, 0 otherwise
     */
    template<class DevIface>
    const DevIface *as() const
    {
        return qobject_cast<const DevIface *>(this);
    }

Q_SIGNALS:
    /**
     * This signal is emitted when the device's link status changed.
     *
     * @param newstate the new state of the connection
     * @param oldstate the previous state of the connection
     * @param reason the reason for the state change, if any.  ReasonNone where the backend
     * provides no reason.
     * @see Device::State
     * @see Device::StateChangeReason
     */
    void stateChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);

    /**
     * Emitted when the autoconnect of this network has changed.
     */
    void activeConnectionChanged();

    /**
     * Emitted when the autoconnect of this network has changed.
     */
    void autoconnectChanged();

    /**
     * Emitted when the list of avaiable connections of this network has changed.
     */
    void availableConnectionChanged();

    /**
     * Emitted when a new connection is available
     */
    void availableConnectionAppeared(const QString &connection);

    /**
     * Emitted when the connection is no longer available
     */
    void availableConnectionDisappeared(const QString &connection);

    /**
     * Emitted when the capabilities of this network has changed.
     */
    void capabilitiesChanged();

    /**
     * Emitted when the DHCP configuration for IPv4 of this network has changed.
     */
    void dhcp4ConfigChanged();

    /**
     * Emitted when the DHCP configuration for IPv6 of this network has changed.
     */
    void dhcp6ConfigChanged();

    /**
     * Emitted when the driver of this network has changed.
     */
    void driverChanged();

    /**
     * Emitted when the driver version of this network has changed.
     */
    void driverVersionChanged();

    /**
     * Emitted when the firmware missing state of this network has changed.
     */
    void firmwareMissingChanged();

    /**
     * Emitted when the firmware version of this network has changed.
     */
    void firmwareVersionChanged();

    /**
     * Emitted when the interface name of this network has changed.
     */
    void interfaceNameChanged();

    /**
     * Emitted when the IPv4 address of this network has changed.
     */
    void ipV4AddressChanged();

    /**
     * Emitted when the IPv4 configuration of this network has changed.
     */
    void ipV4ConfigChanged();

    /**
     * Emitted when the IPv6 configuration of this network has changed.
     */
    void ipV6ConfigChanged();

    /**
     * Emitted when the ip interface name of this network has changed.
     */
    void ipInterfaceChanged();

    /**
     * Emitted when the managed state of this network has changed.
     */
    void managedChanged();

    /**
      * Emitted when the up or down state of the device
      * @since 1.22
      * @note will always return NM_DEVICE_INTERFACE_FLAG_NONE when runtime NM < 1.22
      */
    void interfaceFlagsChanged();

    /**
     * Emitted when the physical port ID changes.
     * @see physicalPortId()
     * @since 0.9.9.0
     */
    void physicalPortIdChanged();

    /**
     * Emitted when the maximum transmission unit has changed
     * @since 0.9.9.0
     */
    void mtuChanged();

    /**
     * Emitted when NmPluginMissing property has changed
     * @since 5.14.0
     * @see nmPluginMissing
     */
    void nmPluginMissingChanged(bool nmPluginMissing);

    /**
     * Emitted when metered property has changed
     * @since 5.14.0
     * @see metered
     */
    void meteredChanged(MeteredStatus metered);

    /**
     * Emitted when the connection state of this network has changed.
     */
    void connectionStateChanged();

    /**
     * Emitted when the state reason of this network has changed.
     */
    void stateReasonChanged();

    /**
     * Emitted when the Unique Device Identifier of this device has changed.
     */
    void udiChanged();

protected:
    NETWORKMANAGERQT_NO_EXPORT Device(DevicePrivate &dd, QObject *parent);

    DevicePrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(Device)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Device::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(Device::Types)
Q_DECLARE_OPERATORS_FOR_FLAGS(Device::Interfaceflags)

class NETWORKMANAGERQT_EXPORT DeviceStateReason
{
public:
    DeviceStateReason(Device::State state, Device::StateChangeReason reason);
    DeviceStateReason(const DeviceStateReason &);
    ~DeviceStateReason();
    Device::State state() const;
    Device::StateChangeReason reason() const;
    DeviceStateReason &operator=(const DeviceStateReason &);

private:
    Q_DECLARE_PRIVATE(DeviceStateReason)

    DeviceStateReasonPrivate *const d_ptr;
};

}

#endif
