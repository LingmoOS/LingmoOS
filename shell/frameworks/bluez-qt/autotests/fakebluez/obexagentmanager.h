/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef OBEXAGENTMANAGER_H
#define OBEXAGENTMANAGER_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class ObexAgentManager : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.AgentManager1")

public:
    explicit ObexAgentManager(QObject *parent = nullptr);

    void runAction(const QString &actionName, const QVariantMap &properties);

public Q_SLOTS:
    void RegisterAgent(const QDBusObjectPath &path, const QDBusMessage &msg);
    void UnregisterAgent(const QDBusObjectPath &path, const QDBusMessage &msg);

private:
    void runAuthorizePushAction(const QVariantMap &properties);
    void runCancelAction();
    void runReleaseAction();

    QDBusObjectPath m_agent;
    QString m_service;
};

#endif // OBEXAGENTMANAGER_H
