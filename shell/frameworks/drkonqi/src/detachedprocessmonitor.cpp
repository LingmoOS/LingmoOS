/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "detachedprocessmonitor.h"
#include "drkonqi_debug.h"

#include <chrono>
#include <errno.h>
#include <signal.h>

#include <QTimerEvent>

using namespace std::chrono_literals;

DetachedProcessMonitor::DetachedProcessMonitor(QObject *parent)
    : QObject(parent)
    , m_pid(0)
{
}

void DetachedProcessMonitor::startMonitoring(int pid)
{
    m_pid = pid;
    startTimer(10ms);
}

void DetachedProcessMonitor::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(m_pid != 0);
    if (::kill(m_pid, 0) < 0) {
        qCDebug(DRKONQI_LOG) << "Process" << m_pid << "finished. kill(2) returned errno:" << errno;
        killTimer(event->timerId());
        m_pid = 0;
        Q_EMIT processFinished();
    }
}

#include "moc_detachedprocessmonitor.cpp"
