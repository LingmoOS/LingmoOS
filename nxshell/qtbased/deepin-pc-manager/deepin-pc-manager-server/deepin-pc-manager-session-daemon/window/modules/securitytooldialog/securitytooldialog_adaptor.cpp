// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitytooldialog_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class SecuritytooldialogAdaptor
 */

SecuritytooldialogAdaptor::SecuritytooldialogAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

SecuritytooldialogAdaptor::~SecuritytooldialogAdaptor()
{
    // destructor
}

QStringList SecuritytooldialogAdaptor::GetInstallingApps()
{
    // handle method call com.deepin.pc.manager.securitytooldialog.GetInstallingApps
    QStringList installingapps;
    QMetaObject::invokeMethod(parent(), "GetInstallingApps", Q_RETURN_ARG(QStringList, installingapps));
    return installingapps;
}

QStringList SecuritytooldialogAdaptor::GetUninstallingApps()
{
    // handle method call com.deepin.pc.manager.securitytooldialog.GetUninstallingApps
    QStringList uninstallingapps;
    QMetaObject::invokeMethod(parent(), "GetUninstallingApps", Q_RETURN_ARG(QStringList, uninstallingapps));
    return uninstallingapps;
}

QStringList SecuritytooldialogAdaptor::GetUpdatingApps()
{
    // handle method call com.deepin.pc.manager.securitytooldialog.GetUpdatingApps
    QStringList updatingapps;
    QMetaObject::invokeMethod(parent(), "GetUpdatingApps", Q_RETURN_ARG(QStringList, updatingapps));
    return updatingapps;
}

void SecuritytooldialogAdaptor::OnInstallPackage(const QString &strPackageKey)
{
    // handle method call com.deepin.pc.manager.securitytooldialog.OnInstallPackage
    QMetaObject::invokeMethod(parent(), "OnInstallPackage", Q_ARG(QString, strPackageKey));
}

void SecuritytooldialogAdaptor::OnRequestUpdateToolsInfo()
{
    // handle method call com.deepin.pc.manager.securitytooldialog.OnRequestUpdateToolsInfo
    QMetaObject::invokeMethod(parent(), "OnRequestUpdateToolsInfo");
}

void SecuritytooldialogAdaptor::OnShowTool(const QString &strPackageKey, const QStringList &strParams)
{
    // handle method call com.deepin.pc.manager.securitytooldialog.OnShowTool
    QMetaObject::invokeMethod(parent(), "OnShowTool", Q_ARG(QString, strPackageKey), Q_ARG(QStringList, strParams));
}

void SecuritytooldialogAdaptor::OnUnInstallPackage(const QString &strPackageKey)
{
    // handle method call com.deepin.pc.manager.securitytooldialog.OnUnInstallPackage
    QMetaObject::invokeMethod(parent(), "OnUnInstallPackage", Q_ARG(QString, strPackageKey));
}

void SecuritytooldialogAdaptor::OnUpdatePackage(const QString &strPackageKey)
{
    // handle method call com.deepin.pc.manager.securitytooldialog.OnUpdatePackage
    QMetaObject::invokeMethod(parent(), "OnUpdatePackage", Q_ARG(QString, strPackageKey));
}

void SecuritytooldialogAdaptor::SetDefenderPaletteType(int type)
{
    // handle method call com.deepin.pc.manager.securitytooldialog.SetDefenderPaletteType
    QMetaObject::invokeMethod(parent(), "SetDefenderPaletteType", Q_ARG(int, type));
}

void SecuritytooldialogAdaptor::ShowNetCheck()
{
    // handle method call com.deepin.pc.manager.securitytooldialog.ShowNetCheck
    QMetaObject::invokeMethod(parent(), "ShowNetCheck");
}

void SecuritytooldialogAdaptor::ShowStartup()
{
    // handle method call com.deepin.pc.manager.securitytooldialog.ShowStartup
    QMetaObject::invokeMethod(parent(), "ShowStartup");
}

