// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleanerdbusadaptorimpl.h"

#include "datainterface_interface.h"
#include "monitornetflow_interface.h"

#include "../../deepin-defender/src/window/modules/common/gsettingkey.h"

#include <QGSettings>

CleanerDBusAdaptorImpl::CleanerDBusAdaptorImpl(QObject *parent)
    : QObject(parent)
    , m_dataInterFaceServer(nullptr)
    , m_monitorInterFaceServer(nullptr)
{
    m_dataInterFaceServer = new ComDeepinDefenderDatainterfaceInterface("com.deepin.defender.datainterface",
                                                                        "/com/deepin/defender/datainterface",
                                                                        QDBusConnection::sessionBus(), this);
    m_monitorInterFaceServer = new ComDeepinDefenderMonitorNetFlowInterface("com.deepin.defender.MonitorNetFlow",
                                                                            "/com/deepin/defender/MonitorNetFlow",
                                                                            QDBusConnection::systemBus(), this);

    m_gsettings = new QGSettings(DEEPIN_DEFENDER_GSETTING_PATH, QByteArray(), this);
}

QStringList CleanerDBusAdaptorImpl::GetTrashInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        rst = m_dataInterFaceServer->GetTrashInfoList();
    }
    return rst;
}

QStringList CleanerDBusAdaptorImpl::GetCacheInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        rst = m_dataInterFaceServer->GetCacheInfoList();
    }
    return rst;
}

QStringList CleanerDBusAdaptorImpl::GetLogInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        rst = m_dataInterFaceServer->GetLogInfoList();
    }
    return rst;
}

QStringList CleanerDBusAdaptorImpl::GetHistoryInfoList()
{
    QStringList rst;
    if (m_dataInterFaceServer) {
        rst = m_dataInterFaceServer->GetHistoryInfoList();
    }
    return rst;
}

QString CleanerDBusAdaptorImpl::GetAppTrashInfoList()
{
    QString rst;
    if (m_dataInterFaceServer) {
        rst = m_dataInterFaceServer->GetAppTrashInfoList();
    }
    return rst;
}

QString CleanerDBusAdaptorImpl::GetBrowserCookiesInfoList()
{
    QString rst;
    if (m_dataInterFaceServer) {
        rst = m_dataInterFaceServer->GetBrowserCookiesInfoList();
    }
    return rst;
}

void CleanerDBusAdaptorImpl::DeleteSpecifiedFiles(QStringList paths)
{
    if (m_dataInterFaceServer) {
        m_dataInterFaceServer->DeleteSpecifiedFiles(paths);
    }
}

void CleanerDBusAdaptorImpl::AddSecurityLog(int nType, const QString &sInfo)
{
    if (m_monitorInterFaceServer) {
        m_monitorInterFaceServer->AddSecurityLog(nType, sInfo);
    }
}

QString CleanerDBusAdaptorImpl::GetValueFromeGSettings(const QString &key)
{
    QString rst;
    if (m_gsettings) {
        rst = m_gsettings->get(key).toString();
    }
    return rst;
}

void CleanerDBusAdaptorImpl::SetValueToGSettings(const QString &key, const QString &value)
{
    if (m_gsettings) {
        m_gsettings->set(key, value);
    }
}
