/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "windowspoller.h"

#include <windows.h>

#include <QTimer>

WindowsPoller::WindowsPoller(QObject *parent)
    : KWindowBasedIdleTimePoller(parent)
{
}

WindowsPoller::~WindowsPoller()
{
}

int WindowsPoller::getIdleTime()
{
#ifndef _WIN32_WCE
    int idle = 0;

    LASTINPUTINFO lii;
    memset(&lii, 0, sizeof(lii));

    lii.cbSize = sizeof(lii);

    BOOL ok = GetLastInputInfo(&lii);
    if (ok) {
        idle = GetTickCount() - lii.dwTime;
    }

    return idle;
#else
    return GetIdleTime();
#endif
}

bool WindowsPoller::additionalSetUp()
{
    m_idleTimer = new QTimer(this);
    connect(m_idleTimer, &QTimer::timeout, this, &WindowsPoller::checkForIdle);
    return true;
}

void WindowsPoller::simulateUserActivity()
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    int x = (int)100 * 65536 / width;
    int y = (int)100 * 65536 / height;

    mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, x, y, NULL, NULL);
}

void WindowsPoller::catchIdleEvent()
{
    m_idleTimer->start(800);
}

void WindowsPoller::stopCatchingIdleEvents()
{
    m_idleTimer->stop();
}

void WindowsPoller::checkForIdle()
{
    if (getIdleTime() < 1000) {
        stopCatchingIdleEvents();
        Q_EMIT resumingFromIdle();
    }
}

#include "moc_windowspoller.cpp"
