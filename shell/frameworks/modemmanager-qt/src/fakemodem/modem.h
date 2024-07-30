/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_H

#include "generictypes.h"

#include <QObject>

#include <QDBusObjectPath>

class Modem : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem")
public:
    explicit Modem(QObject *parent = nullptr);
    ~Modem() override;

    Q_PROPERTY(uint AccessTechnologies READ accessTechnologies)
    Q_PROPERTY(QList<QDBusObjectPath> Bearers READ bearers)
    Q_PROPERTY(ModemManager::UIntList CurrentBands READ currentBands)
    Q_PROPERTY(uint CurrentCapabilities READ currentCapabilities)
    Q_PROPERTY(ModemManager::CurrentModesType CurrentModes READ currentModes)
    Q_PROPERTY(QString Device READ device)
    Q_PROPERTY(QString DeviceIdentifier READ deviceIdentifier)
    Q_PROPERTY(QStringList Drivers READ drivers)
    Q_PROPERTY(QString EquipmentIdentifier READ equipmentIdentifier)
    Q_PROPERTY(QString Manufacturer READ manufacturer)
    Q_PROPERTY(uint MaxActiveBearers READ maxActiveBearers)
    Q_PROPERTY(uint MaxBearers READ maxBearers)
    Q_PROPERTY(QString Model READ model)
    Q_PROPERTY(QStringList OwnNumbers READ ownNumbers)
    Q_PROPERTY(QString Plugin READ plugin)
    Q_PROPERTY(ModemManager::PortList Ports READ ports)
    Q_PROPERTY(uint PowerState READ powerState)
    Q_PROPERTY(QString PrimaryPort READ primaryPort)
    Q_PROPERTY(QString Revision READ revision)
    Q_PROPERTY(ModemManager::SignalQualityPair SignalQuality READ signalQuality)
    Q_PROPERTY(QDBusObjectPath Sim READ sim)
    Q_PROPERTY(int State READ state)
    Q_PROPERTY(uint StateFailedReason READ stateFailedReason)
    Q_PROPERTY(ModemManager::UIntList SupportedBands READ supportedBands)
    Q_PROPERTY(ModemManager::UIntList SupportedCapabilities READ supportedCapabilities)
    Q_PROPERTY(uint SupportedIpFamilies READ supportedIpFamilies)
    Q_PROPERTY(ModemManager::SupportedModesType SupportedModes READ supportedModes)
    Q_PROPERTY(uint UnlockRequired READ unlockRequired)
    Q_PROPERTY(ModemManager::UnlockRetriesMap UnlockRetries READ unlockRetries)

    uint accessTechnologies() const;
    QList<QDBusObjectPath> bearers() const;
    ModemManager::UIntList currentBands() const;
    uint currentCapabilities() const;
    ModemManager::CurrentModesType currentModes() const;
    QString device() const;
    QString deviceIdentifier() const;
    QStringList drivers() const;
    QString equipmentIdentifier() const;
    QString manufacturer() const;
    uint maxActiveBearers() const;
    uint maxBearers() const;
    QString model() const;
    QStringList ownNumbers() const;
    QString plugin() const;
    ModemManager::PortList ports() const;
    uint powerState() const;
    QString primaryPort() const;
    QString revision() const;
    ModemManager::SignalQualityPair signalQuality() const;
    QDBusObjectPath sim() const;
    int state() const;
    uint stateFailedReason() const;
    ModemManager::UIntList supportedBands() const;
    ModemManager::UIntList supportedCapabilities() const;
    uint supportedIpFamilies() const;
    ModemManager::SupportedModesType supportedModes() const;
    uint unlockRequired() const;
    ModemManager::UnlockRetriesMap unlockRetries() const;

    /* Not part of DBus interface */
    void addBearer(const QDBusObjectPath &bearer);
    void removeBearer(const QDBusObjectPath &bearer);
    QString modemPath() const;
    void setEnableNotifications(bool enable);
    void setAccessTechnologies(uint technologies);
    void setDevice(const QString &device);
    void setDeviceIdentifier(const QString &deviceIdentifier);
    void setDrivers(const QStringList &drivers);
    void setEquipmentIdentifier(const QString &identifier);
    void setManufacturer(const QString &manufacturer);
    void setMaxActiveBearers(uint bearers);
    void setMaxBearers(uint bearers);
    void setModemPath(const QString &path);
    void setModel(const QString &model);
    void setOwnNumbers(const QStringList &numbers);
    void setPlugin(const QString &plugin);
    void setPorts(const ModemManager::PortList &ports);
    void setPrimaryPort(const QString &port);
    void setRevision(const QString &revision);
    void setSignalQuality(const ModemManager::SignalQualityPair &signalQuality);
    void setSim(const QDBusObjectPath &sim);
    void setState(int state);
    void setStateFailedReason(uint reason);
    void setSupportedBands(const ModemManager::UIntList &bands);
    void setSupportedCapabilities(const ModemManager::UIntList &capabilities);
    void setSupportedIpFamilies(uint families);
    void setSupportedModes(const ModemManager::SupportedModesType &modes);
    void setUnlockRequired(uint unlockRequired);
    void setUnlockRetries(const ModemManager::UnlockRetriesMap &unlockRetries);

    QVariantMap toMap() const;

public Q_SLOTS:
    Q_SCRIPTABLE QString Command(const QString &cmd, uint timeout);
    Q_SCRIPTABLE QDBusObjectPath CreateBearer(const QVariantMap &properties);
    Q_SCRIPTABLE void DeleteBearer(const QDBusObjectPath &bearer);
    Q_SCRIPTABLE void Enable(bool enable);
    Q_SCRIPTABLE void FactoryReset(const QString &code);
    Q_SCRIPTABLE QList<QDBusObjectPath> ListBearers();
    Q_SCRIPTABLE void Reset();
    Q_SCRIPTABLE void SetCurrentBands(const QList<uint> &bands);
    Q_SCRIPTABLE void SetCurrentCapabilities(uint capabilities);
    Q_SCRIPTABLE void SetCurrentModes(ModemManager::CurrentModesType modes);
    Q_SCRIPTABLE void SetPowerState(uint state);

Q_SIGNALS:
    Q_SCRIPTABLE void StateChanged(int oldState, int newState, uint reason);

private:
    uint m_accessTechnologies;
    QList<QDBusObjectPath> m_bearers;
    ModemManager::UIntList m_currentBands;
    uint m_currentCapabilities;
    ModemManager::CurrentModesType m_currentModes;
    QString m_device;
    QString m_deviceIdentifier;
    QStringList m_drivers;
    bool m_enabledNotifications;
    QString m_equipmentIdentifier;
    QString m_manufacturer;
    uint m_maxActiveBearers;
    uint m_maxBearers;
    QString m_path;
    QString m_model;
    QStringList m_ownNumbers;
    QString m_plugin;
    ModemManager::PortList m_ports;
    uint m_powerState;
    QString m_primaryPort;
    QString m_revision;
    ModemManager::SignalQualityPair m_signalQuality;
    QDBusObjectPath m_sim;
    int m_state;
    uint m_stateFailedReason;
    ModemManager::UIntList m_supportedBands;
    ModemManager::UIntList m_supportedCapabilities;
    uint m_supportedIpFamilies;
    ModemManager::SupportedModesType m_supportedModes;
    uint m_unlockRequired;
    ModemManager::UnlockRetriesMap m_unlockRetries;
};

#endif
