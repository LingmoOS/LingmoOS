// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "performancemonitor.h"

const QString LOG_FLAG = "[PerformanceMonitor]";

const QString GRAB_POINT_INIT_APP_TIME  = "[GRABPOINT] POINT-01";
const QString GRAB_POINT_CLOSE_APP_TIME = "[GRABPOINT] POINT-02";
const QString GRAB_POINT_OPEN_FILE_TIME = "[GRABPOINT] POINT-04";

qint64 PerformanceMonitor::initializeAppStartMs  = 0;
qint64 PerformanceMonitor::inittalizeApoFinishMs = 0;
qint64 PerformanceMonitor::closeAppStartMs       = 0;
qint64 PerformanceMonitor::closeAppFinishMs      = 0;
qint64 PerformanceMonitor::openFileStartMs       = 0;
qint64 PerformanceMonitor::openFileFinishMs      = 0;

PerformanceMonitor::PerformanceMonitor()
{

}

void PerformanceMonitor::initializeAppStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << qPrintable(LOG_FLAG)
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to initialize app";
    initializeAppStartMs = current.toMSecsSinceEpoch();
}

void PerformanceMonitor::initializAppFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << qPrintable(LOG_FLAG)
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to initialize app";

    inittalizeApoFinishMs = current.toMSecsSinceEpoch();
    qint64 time = inittalizeApoFinishMs - initializeAppStartMs;
    qInfo() << qPrintable(QString("%1 startduration=%2ms #(Init app time)").arg(GRAB_POINT_INIT_APP_TIME).arg(time));
}

void PerformanceMonitor::closeAppStart()
{
    QDateTime current = QDateTime::currentDateTime();
    closeAppStartMs = current.toMSecsSinceEpoch();
}

void PerformanceMonitor::closeAPPFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << qPrintable(LOG_FLAG)
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to close app";

    closeAppFinishMs = current.toMSecsSinceEpoch();
    qint64 time = closeAppFinishMs - closeAppStartMs;
    qInfo() << qPrintable(QString("%1 closeduration=%2ms #(Close app time)").arg(GRAB_POINT_CLOSE_APP_TIME).arg(time));
}

void PerformanceMonitor::openFileStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << qPrintable(LOG_FLAG)
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to open file";
    openFileStartMs = current.toMSecsSinceEpoch();
}

void PerformanceMonitor::openFileFinish(const QString &strFileName, qint64 iFileSize)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << qPrintable(LOG_FLAG)
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to open file";

    openFileFinishMs = current.toMSecsSinceEpoch();
    qint64 time = openFileFinishMs - openFileStartMs;
    float fFilesize = iFileSize;
    qInfo() << qPrintable(QString("%1 filename=%2 filezise=%3M opentime=%4ms #(Open file time)").arg(GRAB_POINT_OPEN_FILE_TIME).arg(strFileName).arg(QString::number(fFilesize/(1024*1024), 'f', 6)).arg(time));
}
