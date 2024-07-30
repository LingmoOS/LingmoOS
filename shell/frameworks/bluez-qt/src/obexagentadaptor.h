/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXAGENTADAPTOR_H
#define BLUEZQT_OBEXAGENTADAPTOR_H

#include <QDBusAbstractAdaptor>

#include "request.h"
#include "types.h"

class QDBusObjectPath;
class QDBusPendingCallWatcher;

namespace BluezQt
{
class ObexAgent;
class ObexManager;

class ObexAgentAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Agent1")

public:
    explicit ObexAgentAdaptor(ObexAgent *parent, ObexManager *manager);

public Q_SLOTS:
    QString AuthorizePush(const QDBusObjectPath &transfer, const QDBusMessage &msg);

    Q_NOREPLY void Cancel();
    Q_NOREPLY void Release();

private Q_SLOTS:
    void getPropertiesFinished(QDBusPendingCallWatcher *watcher);

private:
    ObexAgent *m_agent;
    ObexManager *m_manager;
    QString m_transferPath;
    Request<QString> m_transferRequest;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXAGENTADAPTOR_H
