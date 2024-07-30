/*
    SPDX-FileCopyrightText: 2013 Anant Kamath <kamathanant@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMMESSAGING_H
#define MODEMMANAGERQT_MODEMMESSAGING_H

#include <modemmanagerqt_export.h>

#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"
#include "interface.h"
#include "sms.h"

namespace ModemManager
{
class ModemMessagingPrivate;

/**
 * @brief The ModemMessaging class
 *
 * The Messaging interface handles sending SMS messages and notification of new incoming messages.
 */
class MODEMMANAGERQT_EXPORT ModemMessaging : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemMessaging)

public:
    struct Message {
        QString number;
        QString text;
        QByteArray data;
    };

    typedef QSharedPointer<ModemMessaging> Ptr;
    typedef QList<Ptr> List;

    explicit ModemMessaging(const QString &path, QObject *parent = nullptr);
    ~ModemMessaging() override;

    /**
     * @return A list of MMSmsStorage values, specifying the storages supported by this
     * modem for storing and receiving SMS.
     */
    QList<MMSmsStorage> supportedStorages() const;

    /**
     * @return A MMSmsStorage value, specifying the storage to be used when receiving or storing SMS.
     */
    MMSmsStorage defaultStorage() const;

    /**
     * Retrieve all SMS messages.
     *
     * This method should only be used once and subsequent information retrieved
     * either by listening for the messageAdded() signal, or by
     * querying the specific SMS object of interest using findMessage()
     */
    ModemManager::Sms::List messages() const;

    /**
     * Creates a new message object.
     * @param message Message structure with the 'number' and either 'text' or 'data' properties
     */
    QDBusPendingReply<QDBusObjectPath> createMessage(const Message &message);
    /**
     * Creates a new message object.
     * @param message QVariantMap containing message properties
     * The 'number' and either 'text' or 'data' properties are mandatory, others are optional.
     */
    QDBusPendingReply<QDBusObjectPath> createMessage(const QVariantMap &message);

    /**
     * Delete an SMS message.
     *
     * @param uni path to the Sms object
     */
    QDBusPendingReply<void> deleteMessage(const QString &uni);

    /**
     * @param uni path to the Sms object
     * @return pointer to the found Sms (may be null if not found)
     */
    ModemManager::Sms::Ptr findMessage(const QString &uni);

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
    /**
     * Emitted when any part of a new SMS has been received or added (but not
     * for subsequent parts, if any). For messages received from the network,
     * not all parts may have been received and the message may not be
     * complete.
     *
     * Check the 'State' property to determine if the message is complete.
     *
     * @param uni path to the Sms object
     * @param received @p true if the message was received from the network, as opposed to being added locally.
     */
    void messageAdded(const QString &uni, bool received);

    /**
     * Emitted when a message has been deleted.
     * @param uni path to the Sms object
     */
    void messageDeleted(const QString &uni);
};

} // namespace ModemManager

#endif
