/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXOBJECTPUSH_H
#define BLUEZQT_OBEXOBJECTPUSH_H

#include <QObject>

#include "bluezqt_export.h"

#include <memory>

class QDBusObjectPath;

namespace BluezQt
{
class PendingCall;

/**
 * @class BluezQt::ObexObjectPush obexobjectpush.h <BluezQt/ObexObjectPush>
 *
 * OBEX object push.
 *
 * This class represents an OBEX object push interface.
 */
class BLUEZQT_EXPORT ObexObjectPush : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new ObexObjectPush object.
     *
     * This class will be typically used with a @p path
     * from result of ObexManager::createSession().
     *
     * @param path path of session
     * @param parent
     */
    explicit ObexObjectPush(const QDBusObjectPath &path, QObject *parent = nullptr);

    /**
     * Destroys an ObexObjectPush object.
     */
    ~ObexObjectPush() override;

    /**
     * D-Bus object path of the object push session.
     *
     * @return object path of session
     */
    QDBusObjectPath objectPath() const;

    /**
     * Sends one local file to the remote device.
     *
     * The returned ObexTransfer can be used to track progress of transfer.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param fileName full path of file to send
     * @return ObexTransfer * pending call
     */
    PendingCall *sendFile(const QString &fileName);

    /**
     * Pulls the business card from a remote device.
     *
     * If an empty @p targetFileName is given, a name will be
     * automatically calculated for the temporary file.
     *
     * The returned ObexTransfer can be used to track progress of transfer.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param targetFileName full path where the business card will be saved
     * @return ObexTransfer * pending call
     */
    PendingCall *pullBusinessCard(const QString &targetFileName);

    /**
     * Exchanges the business cards on the remote device.
     *
     * This method pushes the local business card to the remote
     * device and then retrieve the remote business card and store
     * it in a local file.
     *
     * If an empty @p targetFileName is given, a name will be
     * automatically calculated for the temporary file.
     *
     * The returned ObexTransfer can be used to track progress of transfer.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param clientFileName full path to local business card
     * @param targetFileName full path where the business card will be saved
     * @return ObexTransfer * pending call
     */
    PendingCall *exchangeBusinessCards(const QString &clientFileName, const QString &targetFileName);

private:
    std::unique_ptr<class ObexObjectPushPrivate> const d;

    friend class ObexObjectPushPrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXOBJECTPUSH_H
