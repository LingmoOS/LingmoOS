// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiscreenmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopWidget>

MultiScreenManager::MultiScreenManager(QObject *parent)
    : QObject(parent)
    , m_registerFunction(nullptr)
    , m_raiseContentFrameTimer(new QTimer(this))
{
    connect(qApp, &QGuiApplication::screenAdded, this, &MultiScreenManager::onScreenAdded, Qt::QueuedConnection);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MultiScreenManager::onScreenRemoved, Qt::QueuedConnection);

    // 在sw平台存在复制模式显示问题，使用延迟来置顶一个Frame
    m_raiseContentFrameTimer->setInterval(50);
    m_raiseContentFrameTimer->setSingleShot(true);

    connect(m_raiseContentFrameTimer, &QTimer::timeout, this, &MultiScreenManager::raiseContentFrame);
}

void MultiScreenManager::register_for_mutil_screen(std::function<QWidget *(QScreen *)> function)
{
    m_registerFunction = function;

    // update all screen
    for (QScreen *screen : qApp->screens()) {
        onScreenAdded(screen);
    }
}

void MultiScreenManager::startRaiseContentFrame()
{
    m_raiseContentFrameTimer->start();
}

void MultiScreenManager::onScreenAdded(QScreen *screen)
{
    if (!m_registerFunction) {
        return;
    }

    m_frames[screen] = m_registerFunction(screen);

    startRaiseContentFrame();
}

void MultiScreenManager::onScreenRemoved(QScreen *screen)
{
    if (!m_registerFunction) {
        return;
    }

    m_frames[screen]->deleteLater();
    m_frames.remove(screen);

    startRaiseContentFrame();
}

void MultiScreenManager::raiseContentFrame()
{
    for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
        if (it.value()->property("contentVisible").toBool()) {
            it.value()->raise();
            return;
        }
    }
}
