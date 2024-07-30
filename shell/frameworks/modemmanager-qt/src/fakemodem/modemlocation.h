/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_LOCATION_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_LOCATION_H

#include "generictypes.h"

#include <QDBusAbstractAdaptor>

class ModemLocation : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Location")
public:
    explicit ModemLocation(QObject *parent = nullptr);
    ~ModemLocation() override;

    Q_PROPERTY(uint Capabilities READ capabilities)
    Q_PROPERTY(uint Enabled READ enabled)
    Q_PROPERTY(ModemManager::LocationInformationMap Location READ location)
    Q_PROPERTY(bool SignalsLocation READ signalsLocation)

    uint capabilities() const;
    uint enabled() const;
    ModemManager::LocationInformationMap location() const;
    bool signalsLocation() const;

    /* Not part of dbus interface */
    void setModemPath(const QString &path);
    void setEnableNotifications(bool enable);
    void setCapabilities(uint capabilities);
    void setEnabled(uint enabled);
    void setLocation(const ModemManager::LocationInformationMap &location);
    void setSignalsLocation(bool signalsLocation);

    QVariantMap toMap() const;

public Q_SLOTS:
    Q_SCRIPTABLE ModemManager::LocationInformationMap GetLocation();
    Q_SCRIPTABLE void Setup(uint sources, bool signal_location);

private:
    QString m_modemPath;
    bool m_enabledNotifications;
    uint m_capabilities;
    uint m_enabled;
    ModemManager::LocationInformationMap m_location;
    bool m_signalsLocation;
};

#endif
