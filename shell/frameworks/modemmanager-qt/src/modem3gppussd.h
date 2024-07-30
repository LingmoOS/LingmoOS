/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM3GPPUSSD_H
#define MODEMMANAGERQT_MODEM3GPPUSSD_H

#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"
#include "interface.h"

namespace ModemManager
{
class Modem3gppUssdPrivate;

/**
 * @brief The Modem3gppUssd class
 *
 * This class provides access to actions based on the USSD protocol.
 */
class MODEMMANAGERQT_EXPORT Modem3gppUssd : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Modem3gppUssd)

public:
    typedef QSharedPointer<Modem3gppUssd> Ptr;
    typedef QList<Ptr> List;

    explicit Modem3gppUssd(const QString &path, QObject *parent = nullptr);
    ~Modem3gppUssd() override;

    /**
     * Sends a USSD @p command string to the network initiating a USSD session.
     *
     * When the request is handled by the network, the method returns the
     * response or an appropriate error. The network may be awaiting further
     * response from the ME after returning from this method and no new command
     * can be initiated until this one is cancelled or ended.
     */
    QDBusPendingReply<QString> initiate(const QString &command);

    /**
     * Respond to a USSD request that is either initiated by the mobile network,
     * or that is awaiting further input after initiate() was called.
     */
    QDBusPendingReply<QString> respond(const QString &response);

    /**
     * Cancel an ongoing USSD session, either mobile or network initiated.
     */
    void cancel();

    /**
     * @return the state of any ongoing USSD session
     */
    MMModem3gppUssdSessionState state() const;

    /**
     * @return any network-initiated request to which no USSD response is required
     *
     * When no USSD session is active, or when there is no network- initiated request, this property will be an empty string.
     */
    QString networkNotification() const;

    /**
     * @return any pending network-initiated request for a response. Client
     * should call respond() with the appropriate response to this request.
     *
     * When no USSD session is active, or when there is no pending
     * network-initiated request, this property will be an empty string.
     */
    QString networkRequest() const;

    /**
     * Sets the timeout in milliseconds for all async method DBus calls.
     * -1 means the default DBus timeout (usually 25 seconds).
     */
    void setTimeout(int timeout);

    /**
     * Returns the current value of the DBus timeout in milliseconds.
     * -1 means the default DBus timeout (usually 25 seconds).
     */
    int timeout() const;

Q_SIGNALS:
    void stateChanged(MMModem3gppUssdSessionState state);
    void networkNotificationChanged(const QString &networkNotification);
    void networkRequestChanged(const QString &networkRequest);
};

} // namespace ModemManager

#endif
