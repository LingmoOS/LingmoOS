//SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later
#include "systeminfointeraction.h"

#include "oceanuifactory.h"

#include <QtQml/qqml.h>

using namespace  oceanuiV25;

SystemInfoInteraction::SystemInfoInteraction(QObject *parent)
    : QObject{ parent }
    , m_systemInfoWork(nullptr)
    , m_systemInfoMode(nullptr)
{
    qmlRegisterType<SystemInfoWork>("org.lingmo.oceanui.systemInfo", 1, 0, "SystemInfoWork");
    qmlRegisterType<SystemInfoModel>("org.lingmo.oceanui.systemInfo", 1, 0, "SystemInfoModel");

    m_systemInfoMode = new SystemInfoModel(this);
    m_systemInfoWork = new SystemInfoWork(m_systemInfoMode, this);

    connect(m_systemInfoWork, &SystemInfoWork::requestUeProgram, this, &SystemInfoInteraction::requestUeProgram);
    m_systemInfoWork->activate();
}

SystemInfoWork *SystemInfoInteraction::systemInfoWork() const
{
    return m_systemInfoWork;
}

void SystemInfoInteraction::setSystemInfoWork(SystemInfoWork *newSystemInfoWork)
{
    m_systemInfoWork = newSystemInfoWork;
}

SystemInfoModel *SystemInfoInteraction::systemInfoMode() const
{
    return m_systemInfoMode;
}

void SystemInfoInteraction::setSystemInfoMode(SystemInfoModel *newSystemInfoMode)
{
    m_systemInfoMode = newSystemInfoMode;
}

DCC_FACTORY_CLASS(SystemInfoInteraction)


#include "systeminfointeraction.moc"
