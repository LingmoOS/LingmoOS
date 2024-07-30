/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXSESSION_P_H
#define BLUEZQT_OBEXSESSION_P_H

#include "obexsession1.h"

namespace BluezQt
{
class ObexSession;

typedef org::bluez::obex::Session1 BluezSession;

class ObexSessionPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ObexSessionPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

    QWeakPointer<ObexSession> q;
    BluezSession *m_bluezSession;

    QString m_source;
    QString m_destination;
    quint8 m_channel;
    QString m_target;
    QString m_root;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXSESSION_P_H
