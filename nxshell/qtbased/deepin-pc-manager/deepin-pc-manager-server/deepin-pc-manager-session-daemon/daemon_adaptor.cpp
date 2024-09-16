// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemon_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class DaemonAdaptor
 */

DaemonAdaptor::DaemonAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DaemonAdaptor::~DaemonAdaptor()
{
    // destructor
}

void DaemonAdaptor::DeleteSpecifiedFiles(const QStringList &path)
{
    // handle method call com.deepin.pc.manager.session.daemon.DeleteSpecifiedFiles
    QMetaObject::invokeMethod(parent(), "DeleteSpecifiedFiles", Q_ARG(QStringList, path));
}

void DaemonAdaptor::ExitApp()
{
    // handle method call com.deepin.pc.manager.session.daemon.ExitApp
    QMetaObject::invokeMethod(parent(), "ExitApp");
}

QString DaemonAdaptor::GetAppTrashInfoList()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetAppTrashInfoList
    QString cleanerPath;
    QMetaObject::invokeMethod(parent(), "GetAppTrashInfoList", Q_RETURN_ARG(QString, cleanerPath));
    return cleanerPath;
}

QString DaemonAdaptor::GetBrowserCookiesInfoList()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetBrowserCookiesInfoList
    QString cleanerPath;
    QMetaObject::invokeMethod(parent(), "GetBrowserCookiesInfoList", Q_RETURN_ARG(QString, cleanerPath));
    return cleanerPath;
}

QStringList DaemonAdaptor::GetCacheInfoList()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetCacheInfoList
    QStringList cleanerPath;
    QMetaObject::invokeMethod(parent(), "GetCacheInfoList", Q_RETURN_ARG(QStringList, cleanerPath));
    return cleanerPath;
}

QStringList DaemonAdaptor::GetHistoryInfoList()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetHistoryInfoList
    QStringList cleanerPath;
    QMetaObject::invokeMethod(parent(), "GetHistoryInfoList", Q_RETURN_ARG(QStringList, cleanerPath));
    return cleanerPath;
}

QStringList DaemonAdaptor::GetLogInfoList()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetLogInfoList
    QStringList cleanerPath;
    QMetaObject::invokeMethod(parent(), "GetLogInfoList", Q_RETURN_ARG(QStringList, cleanerPath));
    return cleanerPath;
}

QString DaemonAdaptor::GetSystemArchitecture()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetSystemArchitecture
    QString arch;
    QMetaObject::invokeMethod(parent(), "GetSystemArchitecture", Q_RETURN_ARG(QString, arch));
    return arch;
}

QStringList DaemonAdaptor::GetTrashInfoList()
{
    // handle method call com.deepin.pc.manager.session.daemon.GetTrashInfoList
    QStringList cleanerPath;
    QMetaObject::invokeMethod(parent(), "GetTrashInfoList", Q_RETURN_ARG(QStringList, cleanerPath));
    return cleanerPath;
}

void DaemonAdaptor::ModulesRequestAuthorityActived()
{
    // handle method call com.deepin.pc.manager.session.daemon.ModulesRequestAuthorityActived
    QMetaObject::invokeMethod(parent(), "ModulesRequestAuthorityActived");
}

void DaemonAdaptor::RequestAuthWithID(const QString &moduleName, int operationID)
{
    // handle method call com.deepin.pc.manager.session.daemon.RequestAuthWithID
    QMetaObject::invokeMethod(parent(), "RequestAuthWithID", Q_ARG(QString, moduleName), Q_ARG(int, operationID));
}

void DaemonAdaptor::RequestCleanSelectTrash()
{
    // handle method call com.deepin.pc.manager.session.daemon.RequestCleanSelectTrash
    QMetaObject::invokeMethod(parent(), "RequestCleanSelectTrash");
}

void DaemonAdaptor::RequestStartTrashScan()
{
    // handle method call com.deepin.pc.manager.session.daemon.RequestStartTrashScan
    QMetaObject::invokeMethod(parent(), "RequestStartTrashScan");
}

void DaemonAdaptor::StartApp()
{
    // handle method call com.deepin.pc.manager.session.daemon.StartApp
    QMetaObject::invokeMethod(parent(), "StartApp");
}

bool DaemonAdaptor::exeAutostart(int nStatus, const QString &sPath)
{
    // handle method call com.deepin.pc.manager.session.daemon.exeAutostart
    bool value;
    QMetaObject::invokeMethod(parent(), "exeAutostart", Q_RETURN_ARG(bool, value), Q_ARG(int, nStatus), Q_ARG(QString, sPath));
    return value;
}

QString DaemonAdaptor::getAppsInfoDisable()
{
    // handle method call com.deepin.pc.manager.session.daemon.getAppsInfoDisable
    QString value;
    QMetaObject::invokeMethod(parent(), "getAppsInfoDisable", Q_RETURN_ARG(QString, value));
    return value;
}

QString DaemonAdaptor::getAppsInfoEnable()
{
    // handle method call com.deepin.pc.manager.session.daemon.getAppsInfoEnable
    QString value;
    QMetaObject::invokeMethod(parent(), "getAppsInfoEnable", Q_RETURN_ARG(QString, value));
    return value;
}

bool DaemonAdaptor::isAutostart(const QString &sPath)
{
    // handle method call com.deepin.pc.manager.session.daemon.isAutostart
    bool value;
    QMetaObject::invokeMethod(parent(), "isAutostart", Q_RETURN_ARG(bool, value), Q_ARG(QString, sPath));
    return value;
}

void DaemonAdaptor::preInitialize()
{
    // handle method call com.deepin.pc.manager.session.daemon.preInitialize
    QMetaObject::invokeMethod(parent(), "preInitialize");
}

bool DaemonAdaptor::startLauncherManage()
{
    // handle method call com.deepin.pc.manager.session.daemon.startLauncherManage
    bool value;
    QMetaObject::invokeMethod(parent(), "startLauncherManage", Q_RETURN_ARG(bool, value));
    return value;
}

