// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

#include "common/thread_manager.h"
#include "system/system_monitor_thread.h"
#include "common/error_context.h"
//#include "process/process_db.h"

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
}

bool Application::event(QEvent *event)
{
    SystemMonitorThread *thread = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread);
    thread->start();

    return DApplication::event(event);
}

MonitorStartEvent::~MonitorStartEvent()
{
}

NetifStartEvent::~NetifStartEvent()
{
}
