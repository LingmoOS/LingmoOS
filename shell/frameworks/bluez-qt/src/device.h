/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_DEVICE_H
#define BLUEZQT_DEVICE_H

#include <QObject>

#include "bluezqt_export.h"
#include "types.h"

#include <memory>

namespace BluezQt
{
class Adapter;
class PendingCall;

/**
 * @class BluezQt::Device device.h <BluezQt/Device>
 *
 * Bluetooth device.
 *
 * This class represents a Bluetooth device.
 */
class BLUEZQT_EXPORT Device : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString ubi READ ubi)
    Q_PROPERTY(QString address READ address NOTIFY addressChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString friendlyName READ friendlyName NOTIFY friendlyNameChanged)
    Q_PROPERTY(QString remoteName READ remoteName NOTIFY remoteNameChanged)
    Q_PROPERTY(quint32 deviceClass READ deviceClass NOTIFY deviceClassChanged)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(quint16 appearance READ appearance NOTIFY appearanceChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool paired READ isPaired NOTIFY pairedChanged)
    Q_PROPERTY(bool trusted READ isTrusted WRITE setTrusted NOTIFY trustedChanged)
    Q_PROPERTY(bool blocked READ isBlocked WRITE setBlocked NOTIFY blockedChanged)
    Q_PROPERTY(bool legacyPairing READ hasLegacyPairing NOTIFY legacyPairingChanged)
    Q_PROPERTY(qint16 rssi READ rssi NOTIFY rssiChanged)
    Q_PROPERTY(ManData manufacturerData READ manufacturerData NOTIFY manufacturerDataChanged)
    Q_PROPERTY(bool servicesResolved READ isServicesResolved NOTIFY servicesResolvedChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList uuids READ uuids NOTIFY uuidsChanged)
    Q_PROPERTY(QString modalias READ modalias NOTIFY modaliasChanged)
    Q_PROPERTY(BatteryPtr battery READ battery NOTIFY batteryChanged)
    Q_PROPERTY(InputPtr input READ input NOTIFY inputChanged)
    Q_PROPERTY(MediaPlayerPtr mediaPlayer READ mediaPlayer NOTIFY mediaPlayerChanged)
    Q_PROPERTY(MediaTransportPtr mediaTransport READ mediaTransport NOTIFY mediaTransportChanged)
    Q_PROPERTY(AdapterPtr adapter READ adapter)
    Q_PROPERTY(QList<GattServiceRemotePtr> gattServices READ gattServices NOTIFY gattServicesChanged)

public:
    /**
     * %Device types.
     */
    enum Type {
        /** The device is a phone. */
        Phone,
        /** The device is a modem. */
        Modem,
        /** The device is a computer. */
        Computer,
        /** The device is a network. */
        Network,
        /** The device is a headset. */
        Headset,
        /** The device is a headphones. */
        Headphones,
        /** The device is an uncategorized audio video device. */
        AudioVideo,
        /** The device is a keyboard. */
        Keyboard,
        /** The device is a mouse. */
        Mouse,
        /** The device is a joypad. */
        Joypad,
        /** The device is a graphics tablet (input device). */
        Tablet,
        /** The device is an uncategorized peripheral device. */
        Peripheral,
        /** The device is a camera. */
        Camera,
        /** The device is a printer. */
        Printer,
        /** The device is an uncategorized imaging device. */
        Imaging,
        /** The device is a wearable device. */
        Wearable,
        /** The device is a toy. */
        Toy,
        /** The device is a health device. */
        Health,
        /** The device is not of any of the known types. */
        Uncategorized,
    };
    Q_ENUM(Type)

    /**
     * Destroys a Device object.
     */
    ~Device() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return DevicePtr
     */
    DevicePtr toSharedPtr() const;

    /**
     * Returns an UBI of the device.
     *
     * Example UBI: "/org/bluez/hci0/dev_40_79_6A_0C_39_75"
     *
     * @return UBI of device
     */
    QString ubi() const;

    /**
     * Returns an address of the device.
     *
     * Example address: "40:79:6A:0C:39:75"
     *
     * @return address of device
     */
    QString address() const;

    /**
     * Returns a name of the device.
     *
     * If the name of the device wasn't previously changed,
     * remoteName is returned.
     *
     * @return name of device
     */
    QString name() const;

    /**
     * Sets the name of the device.
     *
     * @param name name for device
     * @return void pending call
     */
    PendingCall *setName(const QString &name);

    /**
     * Returns a friendly name of the device.
     *
     * Friendly name is a string "name (remoteName)".
     * If the remoteName is same as name, it returns just name.
     *
     * @return friendly name of device
     */
    QString friendlyName() const;

    /**
     * Returns a remote name of the device.
     *
     * @return remote name of device
     */
    QString remoteName() const;

    /**
     * Returns a class of the device.
     *
     * In case of Bluetooth Low Energy only devices,
     * device class is invalid (0).
     *
     * @see type() const
     *
     * @return class of device
     */
    quint32 deviceClass() const;

    /**
     * Returns a type of the device.
     *
     * Type of device is deduced from its class (for Bluetooth Classic devices)
     * or its appearance (for Bluetooth Low Energy devices).
     *
     * @see deviceClass() const
     * @see appearance() const
     *
     * @return type of device
     */
    Device::Type type() const;

    /**
     * Returns an appearance of the device.
     *
     * @return appearance of device
     */
    quint16 appearance() const;

    /**
     * Returns an icon name of the device.
     *
     * In case the icon is empty, "preferences-system-bluetooth" is returned.
     *
     * @return icon name of device
     */
    QString icon() const;

    /**
     * Returns whether the device is paired.
     *
     * @return true if device is paired
     */
    bool isPaired() const;

    /**
     * Returns whether the device is trusted.
     *
     * @return true if device is trusted
     */
    bool isTrusted() const;

    /**
     * Sets the trusted state of the device.
     *
     * @param trusted trusted state
     * @return void pending call
     */
    PendingCall *setTrusted(bool trusted);

    /**
     * Returns whether the device is blocked.
     *
     * @return true if device is blocked
     */
    bool isBlocked() const;

    /**
     * Sets the blocked state of the device.
     *
     * @param blocked blocked state
     * @return void pending call
     */
    PendingCall *setBlocked(bool blocked);

    /**
     * Returns whether the device has legacy pairing.
     *
     * @return true if device has legacy pairing
     */
    bool hasLegacyPairing() const;

    /**
     * Returns Received Signal Strength Indicator of the device.
     *
     * The bigger value indicates better signal strength.
     *
     * @note RSSI is only updated during discovery.
     *
     * @return RSSI of device
     */
    qint16 rssi() const;

    /**
     * Returns manufacturer specific advertisement data.
     *
     * @note Keys are 16 bits Manufacturer ID followed by
     * its byte array value.
     *
     * @return manufacturerData of device.
     */
    ManData manufacturerData() const;

    /**
     * Returns whether or not service discovery has been resolved.
     *
     * @return true if servicesResolved
     */
    bool isServicesResolved() const;

    /**
     * Returns whether the device is connected.
     *
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * Returns UUIDs of supported services by the device.
     *
     * UUIDs will always be returned in uppercase.
     *
     * @return UUIDs of supported services
     */
    QStringList uuids() const;

    /**
     * Returns remote device ID in modalias format.
     *
     * @return device modalias
     */
    QString modalias() const;

    /**
     * Returns the service advertisement data.
     *
     * @returns A hash with keys being the UUIDs in and values being the raw service data value.
     * @since 5.72
     */
    QHash<QString, QByteArray> serviceData() const;

    /**
     * Returns the battery interface for the device.
     *
     * @return null if device has no battery
     * @since 5.66
     */
    BatteryPtr battery() const;

    /**
     * Returns the input interface for the device.
     *
     * Only input devices will have valid input interface.
     *
     * @return null if device have no input
     */
    InputPtr input() const;

    /**
     * Returns the media player interface for the device.
     *
     * Only devices with connected appropriate profile will
     * have valid media player interface.
     *
     * @return null if device have no media player
     */
    MediaPlayerPtr mediaPlayer() const;

    /**
     * Returns the media transport interface for the device.
     *
     * Only devices with connected appropriate profile will
     * have valid media transport interface.
     *
     * @return null if device have no media transport
     */
    MediaTransportPtr mediaTransport() const;

    /**
     * Returns an adapter that discovered this device.
     *
     * @return adapter of device
     */
    AdapterPtr adapter() const;

    /**
     * Returns list of services known by the device.
     *
     * @return list of services
     */
    QList<GattServiceRemotePtr> gattServices() const;
    /**
     * Returns a string for device type.
     *
     * @param type device type
     * @return device type string
     */
    static QString typeToString(Device::Type type);

    /**
     * Returns a device type for string.
     *
     * @param typeString type string
     * @return device type
     */
    static Device::Type stringToType(const QString &typeString);

public Q_SLOTS:
    /**
     * Connects all auto-connectable profiles of the device.
     *
     * This method indicates success if at least one profile was connected.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return void pending call
     */
    PendingCall *connectToDevice();

    /**
     * Disconnects all connected profiles of the device.
     *
     * This method can be used to cancel not-yet finished connectDevice() call.
     *
     * Possible errors: PendingCall::NotConnected
     *
     * @return void pending call
     */
    PendingCall *disconnectFromDevice();

    /**
     * Connects a specific profile of the device.
     *
     * Possible errors: PendingCall::DoesNotExist, PendingCall::AlreadyConnected,
     *                  PendingCall::ConnectFailed
     *
     * @param uuid service UUID
     * @return void pending call
     */
    PendingCall *connectProfile(const QString &uuid);

    /**
     * Disconnects a specific profile of the device.
     *
     * Possible errors: PendingCall::DoesNotExist, PendingCall::Failed,
     *                  PendingCall::NotConnected, PendingCall::NotSupported
     *
     * @param uuid service UUID
     * @return void pending call
     */
    PendingCall *disconnectProfile(const QString &uuid);

    /**
     * Initiates a pairing with the device.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed,
     *                  PendingCall::AlreadyExists, PendingCall::AuthenticationCanceled,
     *                  PendingCall::AuthenticationFailed, PendingCall::AuthenticationRejected,
     *                  PendingCall::AuthenticationTimeout, PendingCall::ConnectionAttemptFailed
     *
     * @return void pending call
     */
    PendingCall *pair();

    /**
     * Cancels a pairing with the device.
     *
     * This method can be used to cancel pairing operation initiated with pair().
     *
     * Possible errors: PendingCall::DoesNotExist, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *cancelPairing();

Q_SIGNALS:
    /**
     * Indicates that the device was removed.
     */
    void deviceRemoved(DevicePtr device);

    /**
     * Indicates that at least one of the device's properties have changed.
     */
    void deviceChanged(DevicePtr device);

    /**
     * Indicates that a new service was added (eg. found by connection).
     */
    void gattServiceAdded(GattServiceRemotePtr service);

    /**
     * Indicates that device GATT services list has changed
     */
    void gattServicesChanged(QList<GattServiceRemotePtr> services);

    /**
     * Indicates that a service was removed.
     */
    void gattServiceRemoved(GattServiceRemotePtr service);

    /**
     * Indicates that at least one of the device's services have changed.
     */
    void gattServiceChanged(GattServiceRemotePtr service);

    /**
     * Indicates that device's name have changed.
     */
    void nameChanged(const QString &name);

    /**
     * Indicates that device's address have changed.
     */
    void addressChanged(const QString &address);

    /**
     * Indicates that device's friendly name have changed.
     */
    void friendlyNameChanged(const QString &friendlyName);

    /**
     * Indicates that device's remote name have changed.
     */
    void remoteNameChanged(const QString &remoteName);

    /**
     * Indicates that device's class have changed.
     */
    void deviceClassChanged(quint32 deviceClass);

    /**
     * Indicates that device's type have changed.
     */
    void typeChanged(Type type);

    /**
     * Indicates that device's appearance have changed.
     */
    void appearanceChanged(quint16 appearance);

    /**
     * Indicates that device's icon have changed.
     */
    void iconChanged(const QString &icon);

    /**
     * Indicates that device's paired state have changed.
     */
    void pairedChanged(bool paired);

    /**
     * Indicates that device's trusted state have changed.
     */
    void trustedChanged(bool trusted);

    /**
     * Indicates that device's blocked state have changed.
     */
    void blockedChanged(bool blocked);

    /**
     * Indicates that device's legacy pairing state have changed.
     */
    void legacyPairingChanged(bool legacyPairing);

    /**
     * Indicates that device's RSSI have changed.
     */
    void rssiChanged(qint16 rssi);

    /**
     * Indicates that device's manufacturer data have changed.
     */
    void manufacturerDataChanged(ManData man);

    /**
     * Indicates that device's servicesResolved state have changed.
     */
    void servicesResolvedChanged(bool servicesResolved);

    /**
     * Indicates that device's connected state have changed.
     */
    void connectedChanged(bool connected);

    /**
     * Indicates that device's UUIDs have changed.
     */
    void uuidsChanged(const QStringList &uuids);

    /**
     * Indicates that device's modalias have changed.
     */
    void modaliasChanged(const QString &modalias);

    /**
     * Indicates that the device's service data has changed.
     * @since 5.72
     */
    void serviceDataChanged(const QHash<QString, QByteArray> &serviceData);

    /**
     * Indicates that device's battery has changed.
     */
    void batteryChanged(BatteryPtr battery);

    /**
     * Indicates that device's input have changed.
     */
    void inputChanged(InputPtr input);

    /**
     * Indicates that device's media player have changed.
     */
    void mediaPlayerChanged(MediaPlayerPtr mediaPlayer);

    /**
     * Indicates that device's media transport have changed.
     */
    void mediaTransportChanged(MediaTransportPtr mediaTransport);

private:
    BLUEZQT_NO_EXPORT explicit Device(const QString &path, const QVariantMap &properties, AdapterPtr adapter);

    std::unique_ptr<class DevicePrivate> const d;

    friend class DevicePrivate;
    friend class ManagerPrivate;
    friend class Adapter;
};

} // namespace BluezQt

Q_DECLARE_METATYPE(BluezQt::ManData)

#endif // BLUEZQT_DEVICE_H
