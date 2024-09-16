// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TIMESYNC1SERVICE_H
#define TIMESYNC1SERVICE_H
#include "dsystemtimetypes_p.h"
#include "dtksystemtime_global.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>
#include <qobject.h>
#include <qscopedpointer.h>
#include <QStringList>

DSYSTEMTIME_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;

class TimeSync1Service : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeTimeSync1")
public:
    explicit TimeSync1Service(QObject *parent = nullptr);
    virtual ~TimeSync1Service();
    // properties
    Q_PROPERTY(QStringList FallbackNTPServers READ fallbackNTPServers);
    Q_PROPERTY(QStringList LinkNTPServers READ linkNTPServers);
    Q_PROPERTY(QStringList SystemNTPServers READ systemNTPServers);
    Q_PROPERTY(qint64 Frequency READ frequency);
    Q_PROPERTY(QString ServerName READ serverName);
    Q_PROPERTY(Address_p ServerAddress READ serverAddress);
    Q_PROPERTY(Message_p NTPMessage READ NTPMessage);
    Q_PROPERTY(quint64 PollIntervalMaxUSec READ pollIntervalMaxUSec);
    Q_PROPERTY(quint64 PollIntervalMinUSec READ pollIntervalMinUSec);
    Q_PROPERTY(quint64 PollIntervalUSec READ pollIntervalUSec);
    Q_PROPERTY(quint64 RootDistanceMaxUSec READ rootDistanceMaxUSec);
    // properties
    QStringList fallbackNTPServers() const;
    QStringList linkNTPServers() const;
    QStringList systemNTPServers() const;
    qint64 frequency() const;
    QString serverName() const;
    Address_p serverAddress() const;
    Message_p NTPMessage() const;
    quint64 pollIntervalMaxUSec() const;
    quint64 pollIntervalMinUSec() const;
    quint64 pollIntervalUSec() const;
    quint64 rootDistanceMaxUSec() const;

private:
    DDBusInterface *m_inter;
    void registerService();
    void unRegisterService();
};

DSYSTEMTIME_END_NAMESPACE
#endif
