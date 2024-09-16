// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

#include "common/thread_manager.h"
#include "system/system_monitor_thread.h"
#include "system/netif_monitor_thread.h"
#include "process/process_db.h"

#include <QEvent>
#include <QMetaType>
#include <QDebug>
#include <QTimer>

using namespace common::core;
using namespace core::system;

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
    qRegisterMetaType<Application::TaskState>("Application::TaskState");
    qRegisterMetaType<pid_t>("pid_t");
    qRegisterMetaType<ErrorContext>("ErrorContext");

    ThreadManager::instance()->attach(new SystemMonitorThread);
    ThreadManager::instance()->attach(new NetifMonitorThread);
}

Application::~Application()
{
}

void Application::setCurrentTaskState(TaskState taskState)
{
    m_currentTaskStat = taskState;
}

Application::TaskState Application::getCurrentTaskState()
{
    return m_currentTaskStat;
}

bool Application::event(QEvent *event)
{
    if (event && event->type() == kMonitorStartEventType) {
        SystemMonitorThread *thread = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread);
        thread->start();
    } else if (event && event->type() == kNetifStartEventType) {
        NetifMonitorThread *thread = ThreadManager::instance()->thread<NetifMonitorThread>(BaseThread::kNetifMonitorThread);
        thread->start();
    }
    return DApplication::event(event);
}

MonitorStartEvent::~MonitorStartEvent()
{
}

NetifStartEvent::~NetifStartEvent()
{
}
