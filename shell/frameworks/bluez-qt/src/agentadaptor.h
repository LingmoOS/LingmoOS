/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_AGENTADAPTOR_H
#define BLUEZQT_AGENTADAPTOR_H

#include <QDBusAbstractAdaptor>

class QDBusMessage;
class QDBusObjectPath;

namespace BluezQt
{
class Manager;
class Agent;

class AgentAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")

public:
    explicit AgentAdaptor(Agent *parent, Manager *manager);

public Q_SLOTS:
    QString RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg);
    Q_NOREPLY void DisplayPinCode(const QDBusObjectPath &device, const QString &pincode);
    quint32 RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg);
    Q_NOREPLY void DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint16 entered);
    void RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg);
    void RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &msg);
    void AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &msg);

    Q_NOREPLY void Cancel();
    Q_NOREPLY void Release();

private:
    QString passkeyToString(quint32 passkey) const;

    Agent *m_agent;
    Manager *m_manager;
};

} // namespace BluezQt

#endif // BLUEZQT_AGENTADAPTOR_H
