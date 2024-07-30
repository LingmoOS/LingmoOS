/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_MESSAGING_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_MESSAGING_H

#include "generictypes.h"
#include "sms.h"

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class ModemMessaging : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Messaging")
public:
    explicit ModemMessaging(QObject *parent = nullptr);
    ~ModemMessaging() override;

    Q_PROPERTY(uint DefaultStorage READ defaultStorage)
    Q_PROPERTY(QList<QDBusObjectPath> Messages READ messages)
    Q_PROPERTY(ModemManager::UIntList SupportedStorages READ supportedStorages)

    uint defaultStorage() const;
    QList<QDBusObjectPath> messages() const;
    ModemManager::UIntList supportedStorages() const;

    /* Not part of dbus interface */
    void addMessage(Sms *sms);
    void setModemPath(const QString &path);
    void setEnableNotifications(bool enable);
    void setDefaultStorage(uint defaultStorage);
    void setSupportedStorages(const ModemManager::UIntList &supportedStorages);

    QVariantMap toMap() const;

public Q_SLOTS: // METHODS
    Q_SCRIPTABLE QDBusObjectPath Create(const QVariantMap &properties);
    Q_SCRIPTABLE void Delete(const QDBusObjectPath &path);
    Q_SCRIPTABLE QList<QDBusObjectPath> List();

Q_SIGNALS: // SIGNALS
    Q_SCRIPTABLE void Added(const QDBusObjectPath &path, bool received);
    Q_SCRIPTABLE void Deleted(const QDBusObjectPath &path);

private:
    QString m_modemPath;
    bool m_enabledNotifications;
    int m_messageCounter;
    uint m_defaultStorage;
    QMap<QDBusObjectPath, Sms *> m_messages;
    ModemManager::UIntList m_supportedStorages;
};

#endif
