/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMTIME_H
#define MODEMMANAGERQT_MODEMTIME_H

#include "interface.h"
#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QDateTime>
#include <QObject>
#include <QSharedPointer>

namespace ModemManager
{
class ModemTimePrivate;

/**
 * This class represents the timezone data provided by the network
 */
class MODEMMANAGERQT_EXPORT NetworkTimezone
{
public:
    /**
     * Constructs an empty timezone data object
     */
    NetworkTimezone();

    /**
     * Destroys this NetworkTimezone object.
     */
    ~NetworkTimezone();

    /**
     * Constructs an NetworkTimezone object that is a copy of the object @p other.
     */
    NetworkTimezone(const NetworkTimezone &other);

    /**
     * Returns offset of the timezone from UTC, in minutes (including DST, if applicable)
     */
    int offset() const;

    /**
     * Sets offset of the timezone from UTC
     */
    void setOffset(int offset);

    /**
     * Returns amount of offset that is due to DST (daylight saving time)
     */
    int dstOffset() const;

    /**
     * Sets amount of offset that is due to DST
     */
    void setDstOffset(int dstOffset);

    /**
     * Returns number of leap seconds included in the network time
     */
    int leapSecond() const;

    /**
     * Sets number of leap seconds included in the network timezone
     */
    void setLeapSecond(int leapSecond);
    /**
     * Makes a copy of the NetworkTimezone object @p other.
     */
    NetworkTimezone &operator=(const NetworkTimezone &other);

private:
    class Private;
    Private *const d;
};

/**
 * @brief The ModemTime class
 *
 * This class allows clients to receive network time and timezone updates broadcast by mobile networks.
 */
class MODEMMANAGERQT_EXPORT ModemTime : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemTime)

public:
    typedef QSharedPointer<ModemTime> Ptr;
    typedef QList<Ptr> List;

    explicit ModemTime(const QString &path, QObject *parent = nullptr);
    ~ModemTime() override;

    /**
     * @return the current network time in local time.
     *
     * This method will only work if the modem tracks, or can request, the
     * current network time; it will not attempt to use previously-received
     * network time updates on the host to guess the current network time.
     */
    QDBusPendingReply<QString> networkTime();

    /**
     * @return the timezone data provided by the network.
     * @see NetworkTimezone
     */
    ModemManager::NetworkTimezone networkTimezone() const;

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
     * Sent when the network time is updated.
     * @param dateTime the new date and time
     */
    void networkTimeChanged(const QDateTime &dateTime);
    void networkTimezoneChanged(const ModemManager::NetworkTimezone &timeZone);
};

} // namespace ModemManager

Q_DECLARE_METATYPE(ModemManager::NetworkTimezone)

#endif
