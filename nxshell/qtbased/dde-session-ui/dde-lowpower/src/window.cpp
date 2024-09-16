// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"

#include <QLabel>
#include <QPixmap>
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QWindow>
#include <QTimer>
#include <QGuiApplication>
#include <QScreen>

Window::Window(QWidget *parent)
    : QWidget(parent)
    , m_image(new QLabel(this))
    , m_text(new QLabel(tr("Low battery, please plug in"), this))
    , m_bSleepLock(false)
{
    setAccessibleName("Window");
    m_image->setAccessibleName("LowPowerImage");
    m_text->setAccessibleName("LowPowerText");
    // set window flags as dde-lock, so we can easily cover it.
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

    const qreal ratio = devicePixelRatioF();
    const QString iconPath = qt_findAtNxFile(":/images/lowpower.png", ratio);
    QPixmap pix(iconPath);
    pix.setDevicePixelRatio(ratio);
    m_pix = pix;
    m_image->setPixmap(m_pix);
    m_text->setStyleSheet("QLabel{ color:#ff8000; font-size:15px }");

    setupSize();
    setStyleSheet("Window { background: black }");

    QDBusConnection::sessionBus().connect("org.deepin.dde.LockFront1",
                                              "/org/deepin/dde/LockFront1",
                                              "org.deepin.dde.LockFront1",
                                              "Visible",
                                              this, SLOT(HideShowToRaise(bool)));
}

Window::~Window()
{

}

bool Window::SleepLock()
{
    return m_bSleepLock;
}

void Window::setupSize()
{
    int totalWidth = 0;
    int totalHeight = 0;
    for (const QScreen *screen : qApp->screens()) {
        totalWidth = qMax(totalWidth, screen->geometry().x() + screen->geometry().width());
        totalHeight = qMax(totalHeight, screen->geometry().y() + screen->geometry().height());
    }

    setFixedSize(totalWidth, totalHeight);

    QScreen *screen = qApp->screenAt(QCursor::pos());
    const qreal ratio = screen->devicePixelRatio();
    m_image->setFixedSize(m_pix.size() / ratio);
    m_image->move(screen->geometry().center() - m_image->rect().center());

    m_text->setFixedSize(screen->geometry().width(), 30);
    m_text->setAlignment(Qt::AlignHCenter);
    m_text->move(0, m_image->y() + m_pix.height());

    QDBusInterface inter2Power("org.deepin.dde.Power1",
                               "/org/deepin/dde/Power1",
                               "org.deepin.dde.Power1",
                               QDBusConnection::sessionBus(), this);
    m_bSleepLock =  inter2Power.property("SleepLock").toBool();
}

void Window::HideShowToRaise(bool visible)
{
    if (visible) {
        QTimer::singleShot(200, this, [ = ]  {
            hide();
            this->raise();
            show();
        });
    }
}

LowPowerAdaptor::LowPowerAdaptor(Window * parent) :
    QDBusAbstractAdaptor(parent)
{

}

LowPowerAdaptor::~LowPowerAdaptor()
{

}

void LowPowerAdaptor::Raise()
{
    Window * w = qobject_cast<Window*>(parent());
    if (w) {
        w->raise();
        w->activateWindow();
        w->grabMouse();
        if (!w->SleepLock()) {
           w->grabKeyboard();
        }
    }
}

void LowPowerAdaptor::Quit()
{
    qApp->quit();
}
