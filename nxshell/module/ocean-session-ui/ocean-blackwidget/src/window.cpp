// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"

#include <QApplication>
#include <QDebug>
#include <QWindow>
#include <QTimer>
#include <QGuiApplication>
#include <QScreen>
#include <QDBusConnection>

#include <DConfig>

Window::Window(QWidget *parent)
    : QWidget(parent)
    , m_clickCount(0)
    , m_timer(new QTimer(this))
    , m_gravityRotateBlackEnabled(false)
{
    setAccessibleName("Window");
    move(0, 0);

    // set window flags as ocean-lock, so we can easily cover it.
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus | Qt::FramelessWindowHint);

    if (QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive)) {
        setAttribute(Qt::WA_NativeWindow);
        // 使用最高层级
        windowHandle()->setProperty("_d_dwayland_window-type", "override");
    }
    QCursor cursor(Qt::BlankCursor);
    this->setCursor(cursor);

    DTK_CORE_NAMESPACE::DConfig *dsgConfig = DTK_CORE_NAMESPACE::DConfig::create("org.lingmo.startocean", QString("org.lingmo.Display"));
    if (dsgConfig && dsgConfig->isValid() && dsgConfig->keyList().contains("gravity-rotate-black-enabled")) {
        m_gravityRotateBlackEnabled = dsgConfig->value("gravity-rotate-black-enabled").toBool();
        dsgConfig->deleteLater();
    }
}

Window::~Window()
{

}

void Window::paintBackground(bool show)
{
    if (!show)
        return;

    if (m_gravityRotateBlackEnabled) {
        setFixedSize(10000, 10000);
        setVisible(true);
    } else {
        setupSize();
    }

    setStyleSheet("Window { background: black }");
}

void Window::setupSize()
{
    int totalWidth = 0;
    int totalHeight = 0;
    for (const QScreen *screen : qApp->screens()) {
        totalWidth = qMax(totalWidth, screen->geometry().x() + screen->geometry().width());
        totalHeight = qMax(totalHeight, screen->geometry().y() + screen->geometry().height());
    }

    qInfo() << "Window setup size, totalWidth: "<< totalWidth << ", totalHeight: " << totalHeight;
    setFixedSize(totalWidth, totalHeight);
    setVisible(true);
}

void Window::raiseWindow()
{
    paintBackground();
    raise();
    activateWindow();
    grabMouse();
    grabKeyboard();
    if (!isVisible()) {
        setVisible(true);
    }
}

void Window::setTimer(int interval)
{
    m_timer->setSingleShot(true);
    m_timer->start(interval);
    connect(m_timer, &QTimer::timeout, this, [] {
        qApp->quit();
    });
}

void Window::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_clickCount++;
        if (m_clickCount == 5) {
            setVisible(false);
            m_clickCount = 0;
        }
    }
}

void Window::onNameLost(QString name)
{
    qInfo() << "Window name lost: " << name;
    if (name == "org.lingmo.ocean.BlackScreen1") {
        qApp->quit();
    }
}

BlackWidgetAdaptor::BlackWidgetAdaptor(Window * parent)
    : QDBusAbstractAdaptor(parent)
{

}

BlackWidgetAdaptor::~BlackWidgetAdaptor()
{

}

Window *BlackWidgetAdaptor::parent() const
{
    return qobject_cast<Window *>(QObject::parent());
}

void BlackWidgetAdaptor::Raise()
{
    Window * w = parent();
    if (w) {
        w->raiseWindow();
    }
}

void BlackWidgetAdaptor::Quit()
{
    quitDBusService();
}

void BlackWidgetAdaptor::setActive(bool visible)
{
    auto *w = parent();
    if (w) {
        if (visible) {
            Raise();
            w->setTimer();
        } else {
            releaseGrabDevicesHideBlack();
        }
    }
}

void BlackWidgetAdaptor::quitDBusService()
{
    qInfo() << "Quit DBus service";
    QDBusConnection::sessionBus().unregisterObject("org.lingmo.ocean.BlackScreen1");
    QDBusConnection::sessionBus().unregisterService("org.lingmo.ocean.BlackScreen1");
}

bool BlackWidgetAdaptor::blackScreenVisible()
{
    auto *w = parent();
    return w ? w->isVisible() : false;
}

void BlackWidgetAdaptor::releaseGrabDevicesHideBlack()
{
    qInfo() << Q_FUNC_INFO;
    Window * w = parent();
    if (w) {
        releaseGrabDevices();
        if (w->isVisible())
          w->setVisible(false);
    }
}

void BlackWidgetAdaptor::releaseGrabDevices()
{
    qInfo() << Q_FUNC_INFO;
    Window * w = parent();
    if (w) {
        w->releaseMouse();
        w->releaseKeyboard();
    }
}
