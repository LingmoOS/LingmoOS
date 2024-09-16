// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiscreenmanager.h"
#include "fullscreenbackground.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>

MultiScreenManager::MultiScreenManager(QObject *parent)
    : QObject(parent)
    , m_registerFunction(nullptr)
    , m_raiseContentFrameTimer(new QTimer(this))
    , m_systemDisplay(new SystemDisplayInter("org.deepin.dde.Display1", "/org/deepin/dde/Display1", QDBusConnection::systemBus(), this))
    , m_isCopyMode(false)
{
    connect(qApp, &QGuiApplication::screenAdded, this, &MultiScreenManager::onScreenAdded, Qt::DirectConnection);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MultiScreenManager::onScreenRemoved, Qt::DirectConnection);

    // 在sw平台存在复制模式显示问题，使用延迟来置顶一个Frame
    m_raiseContentFrameTimer->setInterval(50);
    m_raiseContentFrameTimer->setSingleShot(true);

    connect(m_raiseContentFrameTimer, &QTimer::timeout, this, &MultiScreenManager::raiseContentFrame);
    connect(m_systemDisplay, &SystemDisplayInter::ConfigUpdated, this, &MultiScreenManager::onDisplayModeChanged);
    if (m_systemDisplay->isValid()) {
        m_isCopyMode = (COPY_MODE == getDisplayModeByConfig(m_systemDisplay->GetConfig()));
    }
}

void MultiScreenManager::register_for_mutil_screen(std::function<QWidget *(QScreen *, int)> function)
{
    m_registerFunction = function;
    qInfo() << Q_FUNC_INFO << ", is copy mode: " << m_isCopyMode;
    // update all screen
    if (m_isCopyMode) {
        if (!qApp->screens().isEmpty())
            onScreenAdded(qApp->screens().first());
    } else {
        for (QScreen *screen : qApp->screens()) {
            onScreenAdded(screen);
        }
    }
}

void MultiScreenManager::startRaiseContentFrame()
{
    m_raiseContentFrameTimer->start();
}

bool MultiScreenManager::eventFilter(QObject *watched, QEvent *event)
{
    // 捕获lockframe窗口显示事件
    if (event->type() == QEvent::Show) {
        QWidget *widget = qobject_cast<QWidget *>(watched);
        if (widget && m_frames.values().contains(widget)) {
            // 显示的时候获取窗口位置信息（调试）
            QMetaObject::invokeMethod(this, &MultiScreenManager::checkLockFrameLocation, Qt::QueuedConnection);
        }
    }

    return QObject::eventFilter(watched, event);
}

void MultiScreenManager::onScreenAdded(QPointer<QScreen> screen)
{
    qInfo() << Q_FUNC_INFO << ", is copy mode: " << m_isCopyMode << ", screen: " << screen;
    if (!m_registerFunction) {
        return;
    }

    QWidget* w = nullptr;
    if (m_isCopyMode) {
        // 如果m_frames不为空则直接退出
        if (m_frames.isEmpty()) {
            w = m_registerFunction(screen, m_frames.size());
        }
    } else {
        w = m_registerFunction(screen, m_frames.size());
    }

    // 创建全屏窗口的时间较长，可能在此期间屏幕已经被移除了且指针被析构了（手动操作比较难出现，如果显卡或驱动有问题则会出现），
    // 如果指针为空，则不加入Map中，并析构创建的全屏窗口。
    if (w && !screen.isNull()) {
        m_frames[screen] = w;
        w->installEventFilter(this);
    } else if (w) {
        w->deleteLater();
    }

    startRaiseContentFrame();
}

void MultiScreenManager::onScreenRemoved(QPointer<QScreen> screen)
{
    qDebug() << Q_FUNC_INFO << " is copy mode: " << m_isCopyMode << ", screen: " << screen;
    if (!m_registerFunction) {
        return;
    }

    if (m_frames.contains(screen)) {
        if (m_isCopyMode) {
            QWidget *frame = m_frames[screen];
            m_frames.remove(screen);
            // 如果此时m_frames为空则其它的屏幕继续使用此frame，不重新创建
            if (!qApp->screens().isEmpty() && m_frames.isEmpty()) {
                QScreen *screen = qApp->screens().first();
                // 更新frame绑定的屏幕
                m_frames[screen] = frame;
                FullscreenBackground *fullScreenFrame = qobject_cast<FullscreenBackground*>(frame);
                if (fullScreenFrame) {
                    fullScreenFrame->setScreen(screen, true);
                }
            } else {
                frame->deleteLater();
            }
        }else {
            m_frames[screen]->deleteLater();
            m_frames.remove(screen);
        }
    }

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

void MultiScreenManager::onDisplayModeChanged(const QString &)
{
    m_isCopyMode = (COPY_MODE == getDisplayModeByConfig(m_systemDisplay->GetConfig()));

    qInfo() << Q_FUNC_INFO << ", copy mode: " << m_isCopyMode << ", screen size: " << qApp->screens().size();
    if (m_isCopyMode) {
        for (QScreen *screen : qApp->screens()) {
            // 留下一个屏即可
            if (screen != qApp->screens().first())
                onScreenRemoved(screen);
        }
    } else {
        for (QScreen *screen : qApp->screens()) {
            if (!m_frames[screen])
                onScreenAdded(screen);
        }
    }
}

void MultiScreenManager::checkLockFrameLocation()
{
    for (QScreen *screen : m_frames.keys()) {
        if (screen) {
            qInfo() << Q_FUNC_INFO << ", screen:" << screen << " location:" << screen->geometry()
                       << " lockframe:" << m_frames.value(screen) << " location:" << m_frames.value(screen)->geometry();
        }
    }
}

int MultiScreenManager::getDisplayModeByConfig(const QString &config) const
{
    if (config.isEmpty())
        return EXTENDED_MODE;

    const QJsonObject &rootObj = QJsonDocument::fromJson(config.toUtf8()).object();
    if (rootObj.contains("Config")) {
        const QJsonObject &configObj = rootObj.value("Config").toObject();
        if (configObj.contains("DisplayMode")) {
            return configObj.value("DisplayMode").toInt();
        }
    }

    return EXTENDED_MODE;
}
