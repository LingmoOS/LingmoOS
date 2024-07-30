/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_HANDLER_H
#define PLASMA_NM_HANDLER_H

#include "plasmanm_internal_export.h"

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QPointer>
#include <QTimer>

#include <qqmlregistration.h>

#include <ModemManagerQt/GenericTypes>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>

#include <QCoroCore>

class PLASMANM_INTERNAL_EXPORT Handler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Handler(QObject *parent = nullptr);
    ~Handler() override;

    Q_PROPERTY(bool hotspotSupported READ hotspotSupported NOTIFY hotspotSupportedChanged)

    // Not scientifically correct, but a good estimation of whether a scanning is currently in progress.
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)

public:
    bool hotspotSupported() const
    {
        return m_hotspotSupported;
    }

    bool isScanning() const
    {
        return m_ongoingScansCount != 0;
    }

    void addConnection(NMConnection *connection);

public Q_SLOTS:
    /**
     * Activates given connection
     * @connection - d-bus path of the connection you want to activate
     * @device - d-bus path of the device where the connection should be activated
     * @specificParameter - d-bus path of the specific object you want to use for this activation, i.e access point
     */
    void activateConnection(const QString &connection, const QString &device, const QString &specificParameter);
    /**
     * Adds and activates a new wireless connection
     * @device - d-bus path of the wireless device where the connection should be activated
     * @specificParameter - d-bus path of the accesspoint you want to connect to
     * @password - pre-filled password which should be used for the new wireless connection
     * @autoConnect - boolean value whether this connection should be activated automatically when it's available
     *
     * Works automatically for wireless connections with WEP/WPA security, for wireless connections with WPA/WPA
     * it will open the connection editor for advanced configuration.
     * */
    void addAndActivateConnection(const QString &device, const QString &specificParameter, const QString &password = QString());

    /**
     * Request a code that includes the credentials to a said wifi connection
     * Here's some information on how this information is created, it's generally used to put in QR codes to share.
     * https://github.com/zxing/zxing/wiki/Barcode-Contents#wi-fi-network-config-android-ios-11
     *
     * @param connectionPath the d-bus path to the connection we want to read
     * @param ssid the name of the network being displayed
     * @param securityType the authentication protocol used for this specific ssid
     * @param connectionName the name of the connection
     * @see wifiCodeReceived
     */
    void requestWifiCode(const QString &connectionPath,
                         const QString &ssid,
                         /*NetworkManager::WirelessSecurityType*/ int securityType);

    /**
     * Adds a new connection
     * @map - NMVariantMapMap with connection settings
     */
    QCoro::Task<void> addConnection(const NMVariantMapMap &map);

    /**
     * Deactivates given connection
     * @connection - d-bus path of the connection you want to deactivate
     * @device - d-bus path of the connection where the connection is activated
     */
    void deactivateConnection(const QString &connection, const QString &device);
    /**
     * Disconnects all connections
     */
    void disconnectAll();
    void enableAirplaneMode(bool enable);
    void enableNetworking(bool enable);
    void enableWireless(bool enable);

    void enableWwan(bool enable);

    /**
     * Removes given connection
     * @connection - d-bus path of the connection you want to edit
     */
    void removeConnection(const QString &connection);
    /**
     * Updates given connection
     * @connection - connection which should be updated
     * @map - NMVariantMapMap with new connection settings
     */
    QCoro::Task<void> updateConnection(NetworkManager::Connection::Ptr connection, const NMVariantMapMap &map);
    void requestScan(const QString &interface = QString());

    void createHotspot();
    void stopHotspot();

private Q_SLOTS:
    void secretAgentError(const QString &connectionPath, const QString &message);
    void primaryConnectionTypeChanged(NetworkManager::ConnectionSettings::ConnectionType type);
    void unlockRequiredChanged(MMModemLock modemLock);
    void slotRequestWifiCode(QDBusPendingCallWatcher *watcher);

Q_SIGNALS:
    void connectionActivationFailed(const QString &connectionPath, const QString &message);
    void hotspotCreated();
    void hotspotDisabled();
    void hotspotSupportedChanged(bool hotspotSupported);
    void scanningChanged();
    void wifiCodeReceived(const QString &data, const QString &ssid);

private:
    QCoro::Task<void> addAndActivateConnectionDBus(const NMVariantMapMap &map, const QString &device, const QString &specificObject);
    QCoro::Task<void> activateConnectionInternal(const QString &connection, const QString &device, const QString &specificParameter);
    QCoro::Task<void> addAndActivateConnectionInternal(const QString &device, const QString &specificParameter, const QString &password = QString());
    QCoro::Task<void> deactivateConnectionInternal(const QString &connection, const QString &device);
    QCoro::Task<void> removeConnectionInternal(const QString &connection);
    QCoro::Task<void> requestScanInternal(const QString &interface = QString());
    QCoro::Task<void> createHotspotInternal();

    bool m_hotspotSupported;
    bool m_tmpWirelessEnabled;
    bool m_tmpWwanEnabled;
    QString m_tmpConnectionPath;
    QString m_tmpConnectionUuid;
    QString m_tmpDevicePath;
    QString m_tmpSpecificPath;
    QMap<QString, bool> m_bluetoothAdapters;
    QMap<QString, QTimer *> m_wirelessScanRetryTimer;
    short m_ongoingScansCount = 0;

    QCoro::Task<void> enableBluetooth(bool enable);
    void scanRequestFailed(const QString &interface);
    bool checkRequestScanRateLimit(const NetworkManager::WirelessDevice::Ptr &wifiDevice);
    bool checkHotspotSupported();
    void scheduleRequestScan(const QString &interface, int timeout);
    void incrementScansCount();
    void decrementScansCount();

    QPointer<QDBusPendingCallWatcher> m_requestWifiCodeWatcher;
};

#endif // PLASMA_NM_HANDLER_H
