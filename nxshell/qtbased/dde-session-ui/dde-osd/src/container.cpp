// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "container.h"

#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QGSettings>
#include <QScreen>

#include <DPlatformWindowHandle>
#include "org_deepin_dde_display1.h"
#include "org_deepin_dde_display1_monitor.h"
#include "notification/dbusdockinterface.h"

using DisplayInter = org::deepin::dde::Display1;
using MonitorInter = org::deepin::dde::display1::Monitor;

DGUI_USE_NAMESPACE

const int DefaultRadius = 30;

Container::Container(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_quitTimer(new QTimer(this))
    , m_handle(new DPlatformWindowHandle(this))
{
    setAccessibleName("Container");
    setWindowFlags(Qt::ToolTip);

    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_window-type", "tooltip");
    }

    m_quitTimer->setSingleShot(true);
    m_quitTimer->setInterval(60 * 1000);

    m_layout = new QHBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    setLayout(m_layout);

    m_handle->setEnableBlurWindow(true);
    m_handle->setTranslucentBackground(true);
    m_handle->setShadowOffset(QPoint(0, 0));
    m_handle->setBorderWidth(0);

    updateWindowRadius();
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(DBlurEffectWidget::AutoColor);

    connect(m_quitTimer, &QTimer::timeout, this, &Container::onDelayQuit);
}

Container::~Container()
{

}

void Container::setContent(QWidget *content)
{
    m_layout->addWidget(content);
}

void Container::moveToCenter()
{
    QRect displayRect;
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    DisplayInter displayInter("org.deepin.dde.Display1", "/org/deepin/dde/Display1", QDBusConnection::sessionBus());
    QList<QDBusObjectPath> screenList = displayInter.monitors();

    for (const auto &screen : screenList) {
        MonitorInter monitor("org.deepin.dde.Display1", screen.path(), QDBusConnection::sessionBus());
        QRect monitorRect(monitor.x(), monitor.y(), monitor.width(), monitor.height());
        DBusDockInterface dockInter;
        if (monitor.enabled() && monitorRect.contains(dockInter.geometry())) {
            qDebug() << " screen display : " << screen.path();
            displayRect = QRect(monitorRect.x(), monitorRect.y(),
                                monitorRect.width() / ratio, monitorRect.height() / ratio);
            break;
        }
    }

    if (!displayRect.isValid() || displayRect.isEmpty()) {
        const QScreen *screen = qApp->screenAt(QCursor::pos());
        if (screen)
            displayRect = screen->geometry();
    }

    setGeometry(displayRect);
    move(QPoint(displayRect.center().x(), displayRect.bottom() - 180) - QPoint(rect().center().x(), rect().bottom()));
}

void Container::showEvent(QShowEvent *event)
{
    DBlurEffectWidget::showEvent(event);

    m_quitTimer->stop();
}

void Container::hideEvent(QHideEvent *event)
{
    DBlurEffectWidget::hideEvent(event);

    m_quitTimer->start();
}

void Container::updateWindowRadius(int radius)
{
    int value = radius == -1 ? DefaultRadius : radius;

    m_handle->setWindowRadius(value);
}

void Container::onDelayQuit()
{
    const QGSettings gsettings("com.deepin.dde.osd", "/com/deepin/dde/osd/");
    if (gsettings.keys().contains("auto-exit") && gsettings.get("auto-exit").toBool()) {
        if (isVisible())
            return m_quitTimer->start();
        qWarning() << "Killer Timeout, now quiting...";
        qApp->quit();
    }
}
