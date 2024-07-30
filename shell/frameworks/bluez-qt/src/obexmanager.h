/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXMANAGER_H
#define BLUEZQT_OBEXMANAGER_H

#include <QObject>

#include "bluezqt_export.h"
#include "types.h"

#include <memory>

class QDBusObjectPath;

namespace BluezQt
{
class ObexAgent;
class PendingCall;
class InitObexManagerJob;

/**
 * @class BluezQt::ObexManager obexmanager.h <BluezQt/ObexManager>
 *
 * OBEX manager.
 *
 * The entry point to communicate with session BlueZ obex daemon.
 *
 * You must call init() before other functions can be used.
 *
 * @note If manager is not operational, all methods that returns a PendingCall
 *       will fail with PendingCall::InternalError.
 */
class BLUEZQT_EXPORT ObexManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool initialized READ isInitialized)
    Q_PROPERTY(bool operational READ isOperational NOTIFY operationalChanged)
    Q_PROPERTY(QList<ObexSessionPtr> sessions READ sessions)

public:
    /**
     * Creates a new ObexManager object.
     *
     * @param parent
     */
    explicit ObexManager(QObject *parent = nullptr);

    /**
     * Destroys an ObexManager object.
     */
    ~ObexManager() override;

    /**
     * Creates a new init job.
     *
     * @return init manager job
     */
    InitObexManagerJob *init();

    /**
     * Returns whether the manager is initialized.
     *
     * @return true if manager is initialized
     */
    bool isInitialized() const;

    /**
     * Returns whether the manager is operational.
     *
     * The manager is operational when initialization was successful
     * and BlueZ session daemon is running.
     *
     * @return true if manager is operational
     */
    bool isOperational() const;

    /**
     * Returns a list of all sessions.
     *
     * @return list of sessions
     */
    QList<ObexSessionPtr> sessions() const;

    /**
     * Returns a session for specified path.
     *
     * The @p path does not need to be equal to ObexSession path, startsWith
     * test is performed in the search. That means you can use this method
     * to get ObexSession from path returned by createSession().
     *
     * @param path path of session
     * @return null if there is no session with specified path
     */
    ObexSessionPtr sessionForPath(const QDBusObjectPath &path) const;

    /**
     * Attempts to start org.bluez.obex service by D-Bus activation.
     *
     * Possible return values are 1 if the service was started,
     * 2 if the service is already running or error if the service
     * could not be started.
     *
     * @return quint32 pending call
     */
    static PendingCall *startService();

public Q_SLOTS:
    /**
     * Registers agent.
     *
     * This agent will be used to authorize an incoming object push requests.
     *
     * Possible errors: PendingCall::AlreadyExists
     *
     * @param agent agent to be registered
     * @return  void pending call
     */
    PendingCall *registerAgent(ObexAgent *agent);

    /**
     * Unregisters agent.
     *
     * Possible errors: PendingCall::DoesNotExist
     *
     * @param agent agent to be unregistered
     * @return  void pending call
     */
    PendingCall *unregisterAgent(ObexAgent *agent);

    /**
     * Creates a new OBEX session.
     *
     * The @p args parameter is a dictionary to hold optional or
     * type-specific parameters.
     *
     * Typical parameters:
     * <ul>
     *  <li>QString target - type of session to be created</li>
     *  <li>QString source - device address to be used</li>
     * </ul>
     *
     * Supported targets:
     * <ul>
     *   <li>ftp - ObexFileTransfer</li>
     *   <li>map</li>
     *   <li>opp - ObexObjectPush</li>
     *   <li>pbap</li>
     *   <li>sync</li>
     * </ul>
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param destination address of target device
     * @param args session parameters
     * @return QDBusObjectPath pending call
     */
    PendingCall *createSession(const QString &destination, const QVariantMap &args);

    /**
     * Removes an existing OBEX session.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::NotAuthorized
     *
     * @param session session to be removed
     * @return void pending call
     */
    PendingCall *removeSession(const QDBusObjectPath &session);

Q_SIGNALS:
    /**
     * Indicates that operational state have changed.
     */
    void operationalChanged(bool operational);

    /**
     * Indicates that the session was added.
     */
    void sessionAdded(ObexSessionPtr session);

    /**
     * Indicates that the session was removed.
     */
    void sessionRemoved(ObexSessionPtr session);

private:
    std::unique_ptr<class ObexManagerPrivate> const d;

    friend class ObexManagerPrivate;
    friend class InitObexManagerJobPrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXMANAGER_H
