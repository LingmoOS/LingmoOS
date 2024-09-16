// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYSTEMTIME_H
#define DSYSTEMTIME_H

#include "dsystemtimetypes.h"
#include "dtksystemtime_global.h"

#include <dexpected.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qscopedpointer.h>

DSYSTEMTIME_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
class DSystemTimePrivate;

class DSystemTime : public QObject
{
    Q_OBJECT
public:
    explicit DSystemTime(QObject *parent = nullptr);
    virtual ~DSystemTime();
    // properties
    Q_PROPERTY(bool canNTP READ canNTP);
    Q_PROPERTY(bool localRTC READ localRTC);
    Q_PROPERTY(bool NTP READ NTP);
    Q_PROPERTY(bool NTPSynchronized READ NTPSynchronized);
    Q_PROPERTY(QString timezone READ timezone);
    Q_PROPERTY(quint64 RTCTimeUSec READ RTCTimeUSec);
    Q_PROPERTY(QDateTime timeDate READ timeDate);
    bool canNTP() const;
    bool localRTC() const;
    bool NTP() const;
    bool NTPSynchronized() const;
    QString timezone() const;
    quint64 RTCTimeUSec() const;
    QDateTime timeDate() const;
    // sync properties
    Q_PROPERTY(QStringList fallbackNTPServers READ fallbackNTPServers);
    Q_PROPERTY(QStringList linkNTPServers READ linkNTPServers);
    Q_PROPERTY(QStringList systemNTPServers READ systemNTPServers);
    Q_PROPERTY(qint64 frequency READ frequency);
    Q_PROPERTY(QString serverName READ serverName);
    Q_PROPERTY(Address serverAddress READ serverAddress);
    Q_PROPERTY(Message NTPMessage READ NTPMessage);
    Q_PROPERTY(quint64 pollIntervalMaxUSec READ pollIntervalMaxUSec);
    Q_PROPERTY(quint64 pollIntervalMinUSec READ pollIntervalMinUSec);
    Q_PROPERTY(quint64 pollIntervalUSec READ pollIntervalUSec);
    Q_PROPERTY(quint64 rootDistanceMaxUSec READ rootDistanceMaxUSec);
    QStringList fallbackNTPServers() const;
    QStringList linkNTPServers() const;
    QStringList systemNTPServers() const;
    qint64 frequency() const;
    QString serverName() const;
    Address serverAddress() const;
    Message NTPMessage() const;
    quint64 pollIntervalMaxUSec() const;
    quint64 pollIntervalMinUSec() const;
    quint64 pollIntervalUSec() const;
    quint64 rootDistanceMaxUSec() const;
    // slot
public Q_SLOTS:
    DExpected<QStringList> listTimezones() const;
    DExpected<void> setLocalRTC(bool localRTC, bool fixSystem, bool interactive);
    DExpected<void> enableNTP(bool useNTP, bool interactive);
    DExpected<void> setRelativeTime(qint64 usecUTC, bool interactive);
    DExpected<void> setAbsoluteTime(const QDateTime &time, bool interactive);
    DExpected<void> setTimezone(const QString &timezone, bool interactive);

private:
    QScopedPointer<DSystemTimePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DSystemTime)
};

DSYSTEMTIME_END_NAMESPACE
#endif
