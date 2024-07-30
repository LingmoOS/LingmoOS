/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMOMA_H
#define MODEMMANAGERQT_MODEMOMA_H

#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"
#include "interface.h"

namespace ModemManager
{
class ModemOmaPrivate;

/**
 * @brief The ModemManager Open Mobile Alliance interface.
 *
 * This interface allows clients to handle device management operations as specified by the Open Mobile Alliance (OMA).
 *
 * Device management sessions are either on-demand (client-initiated), or automatically initiated by either the device
 * itself or the network.
 *
 * @since 1.1.92
 */
class MODEMMANAGERQT_EXPORT ModemOma : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemOma)
    Q_FLAGS(MMOmaFeature)

public:
    typedef QSharedPointer<ModemOma> Ptr;
    typedef QList<Ptr> List;

    Q_DECLARE_FLAGS(Features, MMOmaFeature)

    explicit ModemOma(const QString &path, QObject *parent = nullptr);
    ~ModemOma() override;

    // properties
    /**
     * @return MMOmaFeature flags, specifying which device management features are enabled or disabled
     */
    Features features() const;

    /**
     * @return list of network-initiated sessions which are waiting to be accepted or rejected, where:
     * @param the first integer is a MMOmaSessionType
     * @param the second integer is the unique session ID.
     */
    OmaSessionTypes pendingNetworkInitiatedSessions() const;

    /**
     * @return type of the current on-going device management session, given as a MMOmaSessionType
     */
    MMOmaSessionType sessionType() const;

    /**
     * @return state of the current on-going device management session, given as a MMOmaSessionState
     */
    MMOmaSessionState sessionState() const;

    // methods
    /**
     * Configures which OMA device management features should be enabled.
     *
     * @param features MMModemOmaFeature flags, specifying which device management features should get enabled or disabled.
     * MM_OMA_FEATURE_NONE will disable all features.
     */
    QDBusPendingReply<void> setup(Features features);

    /**
     * Starts a client-initiated device management session.
     *
     * @param sessionType type of client-initiated device management session,given as a MMOmaSessionType
     */
    QDBusPendingReply<void> startClientInitiatedSession(MMOmaSessionType sessionType);

    /**
     * Accepts or rejects a network-initiated device management session.
     *
     * @param sessionId unique ID of the network-initiated device management session
     * @param accept boolean specifying whether the session is accepted or rejected
     */
    QDBusPendingReply<void> acceptNetworkInitiatedSession(uint sessionId, bool accept);

    /**
     * Cancels the current on-going device management session.
     */
    QDBusPendingReply<void> cancelSession();

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
    void featuresChanged(QFlags<MMOmaFeature> features);
    void pendingNetworkInitiatedSessionsChanged(const ModemManager::OmaSessionTypes &sessions);
    void sessionTypeChanged(MMOmaSessionType sessionType);
    /**
     * Emitted when the session state changed.
     *
     * @param oldState previous session state, given as a MMOmaSessionState
     * @param newState current session state, given as a MMOmaSessionState
     * @param failedReason reason of failure, given as a MMOmaSessionStateFailedReason, if sessionState() is MM_OMA_SESSION_STATE_FAILED
     */
    void sessionStateChanged(MMOmaSessionState oldState, MMOmaSessionState newState, MMOmaSessionStateFailedReason failedReason);
};

} // namespace ModemManager

#endif
