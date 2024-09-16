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

void DaemonAdaptor::CleanJournal()
{
    // handle method call com.deepin.pc.manager.system.daemon.CleanJournal
    QMetaObject::invokeMethod(parent(), "CleanJournal");
}

void DaemonAdaptor::CleanSelectFile(const QStringList &pathList)
{
    // handle method call com.deepin.pc.manager.system.daemon.CleanSelectFile
    QMetaObject::invokeMethod(parent(), "CleanSelectFile", Q_ARG(QStringList, pathList));
}

void DaemonAdaptor::DeleteUninstalledAppRecord(const QString &sAppID)
{
    // handle method call com.deepin.pc.manager.system.daemon.DeleteUninstalledAppRecord
    QMetaObject::invokeMethod(parent(), "DeleteUninstalledAppRecord", Q_ARG(QString, sAppID));
}

void DaemonAdaptor::ExitApp()
{
    // handle method call com.deepin.pc.manager.system.daemon.ExitApp
    QMetaObject::invokeMethod(parent(), "ExitApp");
}

QStringList DaemonAdaptor::GetDiskDevicePathList()
{
    // handle method call com.deepin.pc.manager.system.daemon.GetDiskDevicePathList
    QStringList pathList;
    QMetaObject::invokeMethod(parent(), "GetDiskDevicePathList", Q_RETURN_ARG(QStringList, pathList));
    return pathList;
}

QDBusVariant DaemonAdaptor::GetSysSettingValue(const QString &key)
{
    // handle method call com.deepin.pc.manager.system.daemon.GetSysSettingValue
    QDBusVariant value;
    QMetaObject::invokeMethod(parent(), "GetSysSettingValue", Q_RETURN_ARG(QDBusVariant, value), Q_ARG(QString, key));
    return value;
}

QVariantList DaemonAdaptor::GetUnInstalledApps()
{
    // handle method call com.deepin.pc.manager.system.daemon.GetUnInstalledApps
    QVariantList out0;
    QMetaObject::invokeMethod(parent(), "GetUnInstalledApps", Q_RETURN_ARG(QVariantList, out0));
    return out0;
}

void DaemonAdaptor::InsertUninstalledAppRecord(const QString &sAppID, const QString &sAppName)
{
    // handle method call com.deepin.pc.manager.system.daemon.InsertUninstalledAppRecord
    QMetaObject::invokeMethod(parent(), "InsertUninstalledAppRecord", Q_ARG(QString, sAppID), Q_ARG(QString, sAppName));
}

QString DaemonAdaptor::QueryPackageName(const QString &sBinaryFile)
{
    // handle method call com.deepin.pc.manager.system.daemon.QueryPackageName
    QString sPkgName;
    QMetaObject::invokeMethod(parent(), "QueryPackageName", Q_RETURN_ARG(QString, sPkgName), Q_ARG(QString, sBinaryFile));
    return sPkgName;
}

void DaemonAdaptor::RefreshPackageTable()
{
    // handle method call com.deepin.pc.manager.system.daemon.RefreshPackageTable
    QMetaObject::invokeMethod(parent(), "RefreshPackageTable");
}

bool DaemonAdaptor::SetSysSettingValue(const QString &key, const QDBusVariant &value)
{
    // handle method call com.deepin.pc.manager.system.daemon.SetSysSettingValue
    bool succeed;
    QMetaObject::invokeMethod(parent(), "SetSysSettingValue", Q_RETURN_ARG(bool, succeed), Q_ARG(QString, key), Q_ARG(QDBusVariant, value));
    return succeed;
}

void DaemonAdaptor::StartApp()
{
    // handle method call com.deepin.pc.manager.system.daemon.StartApp
    QMetaObject::invokeMethod(parent(), "StartApp");
}

QString DaemonAdaptor::getProExePath(const QString &sId)
{
    // handle method call com.deepin.pc.manager.system.daemon.getProExePath
    QString value;
    QMetaObject::invokeMethod(parent(), "getProExePath", Q_RETURN_ARG(QString, value), Q_ARG(QString, sId));
    return value;
}

