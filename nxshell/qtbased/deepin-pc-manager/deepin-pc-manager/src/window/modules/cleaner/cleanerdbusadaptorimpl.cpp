// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleanerdbusadaptorimpl.h"

#include "window/modules/common/gsettingkey.h"
#include "window/modules/common/invokers/invokerfactory.h"

CleanerDBusAdaptorImpl::CleanerDBusAdaptorImpl(QObject *parent)
    : QObject(parent)
    , m_dataInterFaceServer(nullptr)
    , m_monitorInterFaceServer(nullptr)
{
    m_dataInterFaceServer =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.session.daemon",
                                                    "/com/deepin/pc/manager/session/daemon",
                                                    "com.deepin.pc.manager.session.daemon",
                                                    ConnectType::SESSION,
                                                    this);
    m_monitorInterFaceServer =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.MonitorNetFlow",
                                                    "/com/deepin/pc/manager/MonitorNetFlow",
                                                    "com.deepin.pc.manager.MonitorNetFlow",
                                                    ConnectType::SYSTEM,
                                                    this);

    m_gsettings = InvokerFactory::GetInstance().CreateSettings(DEEPIN_PC_MANAGER_GSETTING_PATH,
                                                               QByteArray(),
                                                               this);
}

QStringList CleanerDBusAdaptorImpl::GetTrashInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterFaceServer, "GetTrashInfoList");
        GET_MESSAGE_VALUE(QStringList, tmp, msg);
        rst = tmp;
    }
    return rst;
}

QStringList CleanerDBusAdaptorImpl::GetCacheInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterFaceServer, "GetCacheInfoList");
        GET_MESSAGE_VALUE(QStringList, tmp, msg);
        rst = tmp;
    }
    return rst;
}

QStringList CleanerDBusAdaptorImpl::GetLogInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterFaceServer, "GetLogInfoList");
        GET_MESSAGE_VALUE(QStringList, tmp, msg);
        rst = tmp;
    }
    return rst;
}

QStringList CleanerDBusAdaptorImpl::GetHistoryInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterFaceServer, "GetHistoryInfoList");
        GET_MESSAGE_VALUE(QStringList, tmp, msg);
        rst = tmp;
    }
    return rst;
}

QString CleanerDBusAdaptorImpl::GetAppTrashInfoList()
{
    QString rst;
    if (m_dataInterFaceServer) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterFaceServer, "GetAppTrashInfoList");
        GET_MESSAGE_VALUE(QString, tmp, msg);
        rst = tmp;
    }
    return rst;
}

QString CleanerDBusAdaptorImpl::GetBrowserCookiesInfoList()
{
    QString rst;
    if (m_dataInterFaceServer) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterFaceServer, "GetBrowserCookiesInfoList");
        GET_MESSAGE_VALUE(QString, tmp, msg);
        rst = tmp;
    }
    return rst;
}

void CleanerDBusAdaptorImpl::DeleteSpecifiedFiles(QStringList paths)
{
    if (m_dataInterFaceServer) {
        DBUS_NOBLOCK_INVOKE(m_dataInterFaceServer, "DeleteSpecifiedFiles", paths);
    }
}

void CleanerDBusAdaptorImpl::AddSecurityLog(int nType, const QString &sInfo)
{
    if (m_monitorInterFaceServer) {
        DBUS_NOBLOCK_INVOKE(m_monitorInterFaceServer, "AddSecurityLog", nType, sInfo);
    }
}

QString CleanerDBusAdaptorImpl::GetValueFromeGSettings(const QString &key)
{
    QString rst;
    if (m_gsettings) {
        rst = m_gsettings->GetValue(key).toString();
    }
    return rst;
}

void CleanerDBusAdaptorImpl::SetValueToGSettings(const QString &key, const QString &value)
{
    if (m_gsettings) {
        m_gsettings->SetValue(key, value);
    }
}

void CleanerDBusAdaptorImpl::DeleteSpecifiedAppUninstallInfo(const QString &pkgName)
{
    if (m_monitorInterFaceServer) {
        DBUS_NOBLOCK_INVOKE(m_monitorInterFaceServer, "DeleteUninstalledAppRecord", pkgName);
    }
}
