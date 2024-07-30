/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_CALL_H
#define MODEMMANAGERQT_CALL_H

#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"

namespace ModemManager
{
class CallPrivate;

/**
 * Provides an interface to manipulate and control a call
 *
 * Note: MMCallState, MMCallStateReason and MMCallDirection enums are defined in <ModemManager/ModemManager-enums.h>
 * See http://www.freedesktop.org/software/ModemManager/api/latest/ModemManager-Flags-and-Enumerations.html
 */
class MODEMMANAGERQT_EXPORT Call : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Call)

public:
    typedef QSharedPointer<Call> Ptr;
    typedef QList<Ptr> List;

    explicit Call(const QString &path, QObject *parent = nullptr);
    ~Call() override;

    QString uni() const;

    /**
     * Start a call
     */
    QDBusPendingReply<> start();

    /**
     * Accept a call
     */
    QDBusPendingReply<> accept();

    /**
     * Hangup a call
     */
    QDBusPendingReply<> hangup();

    /**
     * Send DTMF
     */
    QDBusPendingReply<> sendDtmf(const QString &dtmf);

    /**
     * This method returns the state of the call
     */
    MMCallState state() const;

    /**
     * This method returns the reason for the call state change
     */
    MMCallStateReason stateReason() const;

    /**
     * This method returns the direction of the call
     */
    MMCallDirection direction() const;

    /**
     * This method returns the remote phone number
     */
    QString number() const;

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
    void stateChanged(MMCallState oldState, MMCallState newState, MMCallStateReason reason);
    void numberChanged(const QString &number);
    void dtmfReceived(const QString &dtmf);

private:
    CallPrivate *const d_ptr;
};

} // namespace ModemManager

#endif
