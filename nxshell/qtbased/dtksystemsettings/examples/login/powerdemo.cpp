// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "powerdemo.h"

#include "dlogintypes.h"

#include <qdebug.h>

#include <iostream>
#include <string>

PowerDemo::PowerDemo(QObject *parent)
    : LoginDemo(parent)
    , m_manager(new DLoginManager)
{
}

int PowerDemo::run()
{
    m_manager->inhibit(InhibitBehavior::Sleep, "demo", "block reboot", InhibitMode::Block);
    m_manager->inhibit(InhibitBehavior::Shutdown, "demo", "delay shutdown", InhibitMode::Delay);
    auto eInhibitors = m_manager->listInhibitors();
    if (eInhibitors) {
        qDebug() << eInhibitors.value();
    } else {
        qDebug() << eInhibitors.error();
    }
    auto eStatus = m_manager->canSuspend();
    if (eStatus) {
        qDebug() << eStatus.value();
    } else {
        qDebug() << eStatus.error();
    }
    eStatus = m_manager->canPowerOff();
    if (eStatus) {
        qDebug() << eStatus.value();
    } else {
        qDebug() << eStatus.error();
    }
    auto time = QDateTime::currentDateTime();
    m_manager->scheduleShutdown(ShutdownType::Reboot, time.addSecs(1000));
    qDebug() << m_manager->scheduledShutdown();
    m_manager->cancelScheduledShutdown();
    qDebug() << m_manager->scheduledShutdown();
    std::string str;
    getline(std::cin, str);
    m_manager->powerOff(false);
    return 0;
}
