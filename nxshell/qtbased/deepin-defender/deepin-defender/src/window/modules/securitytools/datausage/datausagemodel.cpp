// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datausagemodel.h"
#include "window/modules/common/gsettingkey.h"
#include "window/modules/common/invokers/invokerinterface.h"

#include <QFile>
#include <QProcess>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>

DataUsageModel::DataUsageModel(InvokerFactoryInterface *invokerFactory, QObject *parent)
    : QObject(parent)
    , m_monitorInterFaceServer(nullptr)
    , m_dataInterFaceServer(nullptr)
    , m_gsNetControl(nullptr)
{
    m_monitorInterFaceServer = invokerFactory->CreateInvoker("com.deepin.defender.MonitorNetFlow",
                                                             "/com/deepin/defender/MonitorNetFlow",
                                                             "com.deepin.defender.MonitorNetFlow",
                                                             ConnectType::SYSTEM, this);
    m_dataInterFaceServer = invokerFactory->CreateInvoker("com.deepin.defender.datainterface",
                                                             "/com/deepin/defender/datainterface",
                                                             "com.deepin.defender.datainterface",
                                                             ConnectType::SESSION, this);
    m_gsNetControl = invokerFactory->CreateSettings("com.deepin.dde.deepin-defender", QByteArray(), this);

    //链接trafficdetails服务发送的所有应用流量数据信号
    registerDefenderProcInfoMetaType();
    registerDefenderProcInfoListMetaType();
    bool connetRst = true;
    connetRst &= m_monitorInterFaceServer->Connect("SendProcInfoList", this, SLOT(recProcInfoList(DefenderProcInfoList)));
    connetRst &= m_monitorInterFaceServer->Connect("SendAllAppFlowList", this, SLOT(sendAllAppFlowList(int, DefenderProcInfoList)));

    //链接trafficdetails服务发送的应用流量数据信号
    connetRst &= m_monitorInterFaceServer->Connect("SendAppFlowList", this, SLOT(sendAppFlowList(QString, int, DefenderProcInfoList)));
}

QString DataUsageModel::GetNetControlAppStatus() const
{
    return m_gsNetControl->GetValue(NET_CONTROL_APPS_STATUS).toString();
}

bool DataUsageModel::GetDataUsageSwitchStatus() const
{
    return m_gsNetControl->GetValue(DATA_USAGE_ON_OFF).toBool();
}

void DataUsageModel::SetNetAppStatusChange(const QString &pkgName, int index) const
{
    m_dataInterFaceServer->Invoke("setNetAppStatusChange", QList<QVariant>() << pkgName << index, BlockMode::NOBLOCK);
}

void DataUsageModel::AddSecurityLog(const QString &info) const
{
    m_monitorInterFaceServer->Invoke("AddSecurityLog", QList<QVariant>() << SECURITY_LOG_TYPE_TOOL << info, BlockMode::NOBLOCK);
}

DefenderProcInfoList DataUsageModel::getProcInfoList() const
{
    return m_procInfoList;
}

void DataUsageModel::asyncGetAppFlowList(const QString &pkgName, const TimeRangeType &timeRangeType)
{
    m_monitorInterFaceServer->Invoke("AsyncGetAppFlowList", {pkgName, timeRangeType}, BlockMode::NOBLOCK);
}

void DataUsageModel::asyncGetAllAppFlowList(const TimeRangeType &timeRangeType)
{
    m_monitorInterFaceServer->Invoke("AsyncGetAllAppFlowList", {timeRangeType}, BlockMode::NOBLOCK);
}

void DataUsageModel::recProcInfoList(const DefenderProcInfoList &procInfos)
{
    m_procInfoList = procInfos;
    Q_EMIT sendProcInfoList(m_procInfoList);
}
