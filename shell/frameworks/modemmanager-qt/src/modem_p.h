/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM_P_H
#define MODEMMANAGERQT_MODEM_P_H

#include "dbus/modeminterface.h"

#include "bearer.h"
#include "interface_p.h"
#include "modem.h"

namespace ModemManager
{
class ModemPrivate : public InterfacePrivate
{
    Q_OBJECT
public:
    explicit ModemPrivate(const QString &path, Modem *q);
    OrgFreedesktopModemManager1ModemInterface modemIface;

    QMap<QString, Bearer::Ptr> bearers;
    QString simPath;
    QList<MMModemCapability> supportedCapabilities;
    QFlags<MMModemCapability> currentCapabilities;
    uint maxBearers;
    uint maxActiveBearers;
    QString manufacturer;
    QString model;
    QString revision;
    QString deviceIdentifier;
    QString device;
    QStringList drivers;
    QString plugin;
    QString primaryPort;
    PortList ports;
    QString equipmentIdentifier;
    MMModemLock unlockRequired;
    UnlockRetriesMap unlockRetries;
    MMModemState state;
    MMModemStateFailedReason stateFailedReason;
    ModemManager::Modem::AccessTechnologies accessTechnologies;
    SignalQualityPair signalQuality;
    QStringList ownNumbers;
    MMModemPowerState powerState;
    SupportedModesType supportedModes;
    CurrentModesType currentModes;
    QList<MMModemBand> supportedBands;
    QList<MMModemBand> currentBands;
    ModemManager::Modem::IpBearerFamilies supportedIpFamilies;

    ModemManager::Bearer::Ptr findRegisteredBearer(const QString &path);

    Q_DECLARE_PUBLIC(Modem)
    Modem *q_ptr;
private Q_SLOTS:
    void initializeBearers();
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
    void onStateChanged(int oldState, int newState, uint reason);
};

} // namespace ModemManager

#endif
