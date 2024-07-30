/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXSESSION_H
#define BLUEZQT_OBEXSESSION_H

#include <QObject>

#include "bluezqt_export.h"
#include "types.h"

#include <memory>

class QDBusObjectPath;

namespace BluezQt
{
class PendingCall;

/**
 * @class BluezQt::ObexSession obexsession.h <BluezQt/ObexSession>
 *
 * OBEX session.
 *
 * This class represents an OBEX session.
 */
class BLUEZQT_EXPORT ObexSession : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString source READ source)
    Q_PROPERTY(QString destination READ destination)
    Q_PROPERTY(quint8 channel READ channel)
    Q_PROPERTY(QString target READ target)
    Q_PROPERTY(QString root READ root)

public:
    /**
     * Destroys an ObexSession object.
     */
    ~ObexSession() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return ObexSessionPtr
     */
    ObexSessionPtr toSharedPtr() const;

    /**
     * D-Bus object path of the session.
     *
     * @return object path of session
     */
    QDBusObjectPath objectPath() const;

    /**
     * Returns address of the Bluetooth adapter.
     *
     * @see Manager::adapterForAddress() const
     *
     * @return address of adapter
     */
    QString source() const;

    /**
     * Returns address of the Bluetooth device.
     *
     * @see Manager::deviceForAddress() const
     *
     * @return address of device
     */
    QString destination() const;

    /**
     * Returns the Bluetooth channel.
     *
     * @return channel
     */
    quint8 channel() const;

    /**
     * Returns the target UUID.
     *
     * @return target UUID
     */
    QString target() const;

    /**
     * Returns the root path.
     *
     * @return root path
     */
    QString root() const;

    /**
     * Returns the remote device capabilities.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return QString pending call
     */
    PendingCall *getCapabilities();

private:
    BLUEZQT_NO_EXPORT explicit ObexSession(const QString &path, const QVariantMap &properties);

    std::unique_ptr<class ObexSessionPrivate> const d;

    friend class ObexSessionPrivate;
    friend class ObexManagerPrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXSESSION_H
