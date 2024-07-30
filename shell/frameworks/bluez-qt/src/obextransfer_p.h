/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXTRANSFER_P_H
#define BLUEZQT_OBEXTRANSFER_P_H

#include "obextransfer.h"

#include "dbusproperties.h"
#include "obextransfer1.h"

namespace BluezQt
{
typedef org::bluez::obex::Transfer1 BluezTransfer;
typedef org::freedesktop::DBus::Properties DBusProperties;

class ObexTransferPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ObexTransferPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);
    void sessionRemoved(const ObexSessionPtr &session);

    QWeakPointer<ObexTransfer> q;
    BluezTransfer *m_bluezTransfer;
    DBusProperties *m_dbusProperties;

    ObexTransfer::Status m_status;
    QString m_name;
    QString m_type;
    quint64 m_time;
    quint64 m_size;
    quint64 m_transferred;
    QString m_fileName;
    bool m_suspendable;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXTRANSFER_P_H
