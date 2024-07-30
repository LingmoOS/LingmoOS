/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_PENDINGCALL_H
#define BLUEZQT_PENDINGCALL_H

#include <functional>

#include <QObject>

#include "bluezqt_export.h"

#include <memory>

class QDBusError;
class QDBusPendingCall;
class QDBusPendingCallWatcher;

namespace BluezQt
{
/**
 * @class BluezQt::PendingCall pendingcall.h <BluezQt/PendingCall>
 *
 * Pending method call.
 *
 * This class represents a pending method call. It is a convenient wrapper
 * around QDBusPendingReply and QDBusPendingCallWatcher.
 */
class BLUEZQT_EXPORT PendingCall : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value)
    Q_PROPERTY(QVariantList values READ values)
    Q_PROPERTY(int error READ error)
    Q_PROPERTY(QString errorText READ errorText)
    Q_PROPERTY(bool isFinished READ isFinished)
    Q_PROPERTY(QVariant userData READ userData WRITE setUserData)

public:
    /**
     * Known error types.
     */
    enum Error {
        /** Indicates there is no error. */
        NoError = 0,
        /** Indicates that the device is not ready. */
        NotReady = 1,
        /** Indicates that the action have failed. */
        Failed = 2,
        /** Indicates that the action was rejected. */
        Rejected = 3,
        /** Indicates that the action was canceled. */
        Canceled = 4,
        /** Indicates that invalid arguments were passed. */
        InvalidArguments = 5,
        /** Indicates that an agent or pairing record already exists. */
        AlreadyExists = 6,
        /** Indicates that an agent, service or pairing operation does not exists. */
        DoesNotExist = 7,
        /** Indicates that the action is already in progress. */
        InProgress = 8,
        /** Indicates that the action is not in progress. */
        NotInProgress = 9,
        /** Indicates that the device is already connected. */
        AlreadyConnected = 10,
        /** Indicates that the connection to the device have failed. */
        ConnectFailed = 11,
        /** Indicates that the device is not connected. */
        NotConnected = 12,
        /** Indicates that the action is not supported. */
        NotSupported = 13,
        /** Indicates that the caller is not authorized to do the action. */
        NotAuthorized = 14,
        /** Indicates that the authentication was canceled. */
        AuthenticationCanceled = 15,
        /** Indicates that the authentication have failed. */
        AuthenticationFailed = 16,
        /** Indicates that the authentication was rejected. */
        AuthenticationRejected = 17,
        /** Indicates that the authentication timed out. */
        AuthenticationTimeout = 18,
        /** Indicates that the connection attempt have failed. */
        ConnectionAttemptFailed = 19,
        /** Indicates that the data provided generates a data packet which is too long. */
        InvalidLength = 20,
        /** Indicates that the action is not permitted (e.g. maximum reached or socket locked). */
        NotPermitted = 21,
        /** Indicates an error with D-Bus. */
        DBusError = 98,
        /** Indicates an internal error. */
        InternalError = 99,
        /** Indicates an unknown error. */
        UnknownError = 100,
    };
    Q_ENUM(Error)

    /**
     * Destroys a PendingCall object.
     */
    ~PendingCall() override;

    /**
     * Returns a first return value of the call.
     *
     * @return first return value
     */
    QVariant value() const;

    /**
     * Returns all values of the call.
     *
     * @return all return values
     */
    QVariantList values() const;

    /**
     * Returns an error code.
     *
     * @return error code
     * @see Error
     */
    int error() const;

    /**
     * Returns an error text.
     *
     * @return error text
     */
    QString errorText() const;

    /**
     * Returns whether the call is finished.
     *
     * @return true if call is finished
     */
    bool isFinished() const;

    /**
     * Waits for the call to finish.
     *
     * @warning This method blocks until the call finishes!
     */
    void waitForFinished();

    /**
     * Returns the user data of the call.
     *
     * @return user data of call
     */
    QVariant userData() const;

    /**
     * Sets the user data of the call.
     *
     * @param userData user data
     */
    void setUserData(const QVariant &userData);

Q_SIGNALS:
    /**
     * Indicates that the call have finished.
     */
    void finished(PendingCall *call);

private:
    enum ReturnType {
        ReturnVoid,
        ReturnUint32,
        ReturnString,
        ReturnStringList,
        ReturnObjectPath,
        ReturnFileTransferList,
        ReturnTransferWithProperties,
        ReturnByteArray
    };

    BLUEZQT_NO_EXPORT explicit PendingCall(Error error, const QString &errorText, QObject *parent = nullptr);
    BLUEZQT_NO_EXPORT explicit PendingCall(const QDBusPendingCall &call, ReturnType type, QObject *parent = nullptr);

    // exported because called from template BluezQt::TPendingCall constructor
    using ErrorProcessor = std::function<void(const QDBusError &error)>;
    using ExternalProcessor = std::function<void(QDBusPendingCallWatcher *watcher, ErrorProcessor errorProcessor, QVariantList *values)>;
    explicit PendingCall(const QDBusPendingCall &call, ExternalProcessor externalProcessor, QObject *parent = nullptr);

    std::unique_ptr<class PendingCallPrivate> const d;

    friend class PendingCallPrivate;
    friend class Manager;
    friend class Adapter;
    friend class GattServiceRemote;
    friend class GattCharacteristicRemote;
    friend class GattDescriptorRemote;
    friend class Device;
    friend class GattManager;
    friend class LEAdvertisingManager;
    friend class Media;
    friend class MediaPlayer;
    friend class ObexManager;
    friend class ObexTransfer;
    friend class ObexSession;
    friend class ObexObjectPush;
    friend class ObexFileTransfer;
    template<class... T>
    friend class TPendingCall;
};

} // namespace BluezQt

#endif // BLUEZQT_PENDINGCALL_H
