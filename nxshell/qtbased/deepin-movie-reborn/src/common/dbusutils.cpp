// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusutils.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QMutex>

static QMutex mutex;

DBusUtils::DBusUtils()
{

}

QVariant DBusUtils::redDBusProperty(const QString &sService, const QString &sPath, const QString &sInterface, const char *pPropert)
{
    // 创建QDBusInterface接口
    mutex.lock();

    QDBusInterface ainterface(sService, sPath,
                              sInterface,
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
//        qInfo() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        //qInfo() << " QDBusInterface ainterface isValid" << path << propert;
        QVariant v(0) ;
        mutex.unlock();
        return  v;
    }
    //调用远程的value方法
    QVariant v = ainterface.property(pPropert);
    mutex.unlock();
    return  v;
}
QVariant DBusUtils::redDBusMethod(const QString &sService, const QString &sPath, const QString &sInterface, const char *pMethod)
{
    // 创建QDBusInterface接口
    QDBusInterface ainterface(sService, sPath,
                              sInterface,
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        //qInfo() <<  "error:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        QVariant v(0) ;
        return  v;
    }
    //调用远程的value方法
    QDBusReply<QDBusVariant> reply = ainterface.call(pMethod);
    if (reply.isValid()) {
//        return reply.value();
        QVariant v(0) ;
        return  v;
    } else {
        //qInfo() << "error1:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        QVariant v(0) ;
        return  v;
    }
}
