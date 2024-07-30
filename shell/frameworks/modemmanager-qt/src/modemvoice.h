/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMVOICE_H
#define MODEMMANAGERQT_MODEMVOICE_H

#include <modemmanagerqt_export.h>

#include <QObject>
#include <QSharedPointer>

#include "call.h"
#include "generictypes.h"
#include "interface.h"

namespace ModemManager
{
class ModemVoicePrivate;

/**
 * @brief The ModemVoice class
 *
 * The Voice interface handles call related actions
 */
class MODEMMANAGERQT_EXPORT ModemVoice : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemVoice)

public:
    typedef QSharedPointer<ModemVoice> Ptr;
    typedef QList<Ptr> List;

    explicit ModemVoice(const QString &path, QObject *parent = nullptr);
    ~ModemVoice() override;

    /**
     * Retrieve all calls.
     *
     * This method should only be used once and subsequent information retrieved
     * either by listening for the callAdded() signal, or by
     * querying the specific call object of interest using findCall()
     */
    ModemManager::Call::List calls() const;

    /**
     * Creates a new call object.
     * @param number Number property
     */
    QDBusPendingReply<QDBusObjectPath> createCall(const QString &number);

    /**
     * Creates a new call object.
     * @param call QVariantMap containing call properties
     * The 'number' property is mandatory.
     */
    QDBusPendingReply<QDBusObjectPath> createCall(const QVariantMap &call);

    /**
     * Delete a call.
     *
     * @param uni path to the Call object
     */
    QDBusPendingReply<void> deleteCall(const QString &uni);

    /**
     * @param uni path to the Call object
     * @return pointer to the found Call (may be null if not found)
     */
    ModemManager::Call::Ptr findCall(const QString &uni);

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
     * Emitted when an new Call is detected.
     *
     * @param uni path to the Call object
     */
    void callAdded(const QString &uni);

    /**
     * Emitted when a call has been deleted.
     * @param uni path to the Call object
     */
    void callDeleted(const QString &uni);
};

} // namespace ModemManager

#endif
