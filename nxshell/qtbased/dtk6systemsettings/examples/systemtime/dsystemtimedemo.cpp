// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dsystemtime.h"

#include <qdebug.h>
#include <qeventloop.h>
#include <qobject.h>

#include <QCoreApplication>

#include <iostream>

DSYSTEMTIME_USE_NAMESPACE
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DSystemTime systemTime;
    qDebug() << systemTime.serverName();
    qDebug() << systemTime.fallbackNTPServers()[1];
    qDebug() << systemTime.systemNTPServers().isEmpty();
    qDebug() << systemTime.canNTP();
    qDebug() << systemTime.localRTC();
    qDebug() << systemTime.NTP();
    qDebug() << systemTime.NTPSynchronized();
    qDebug() << systemTime.timezone();
    qDebug() << systemTime.RTCTimeUSec();
    qDebug() << systemTime.timeDate();
    qDebug() << systemTime.fallbackNTPServers();
    qDebug() << systemTime.linkNTPServers();
    qDebug() << systemTime.systemNTPServers();
    qDebug() << systemTime.frequency();
    qDebug() << systemTime.serverName();
    systemTime.setTimezone("Asia/Yangon", 1);
}
