// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "suspenddialog.h"

#include <DApplication>

#include <QScreen>
#include <QDebug>
#include <QAbstractButton>
#include "org_deepin_dde_display1.h"
#include "org_deepin_dde_display1_monitor.h"

using MonitorInter = org::deepin::dde::display1::Monitor;

SuspendDialog::SuspendDialog(QRect screenGeometry)
    : DDialog(tr("External monitor detected, suspend?"), tr("%1s").arg(15)),
      m_screenGeometry(screenGeometry),
      m_timerTick(0)
{
    setAccessibleName("SuspendDialog");
    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    const auto ratio = devicePixelRatioF();
    QPixmap icon = QIcon::fromTheme("computer").pixmap(QSize(48, 48) * ratio);
    icon.setDevicePixelRatio(ratio);

    setIcon(icon);

    QStringList buttons;
    buttons << tr("Cancel") << tr("Suspend");
    addButtons(buttons);
    getButton(0)->setFocus();

    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        m_timerTick++;
        if (m_timerTick >= 15) {
            done(1);
        } else {
            setMessage(tr("%1s").arg(15 - m_timerTick));
        }
    });

    m_timer.start();
}

void SuspendDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        const auto ratio = devicePixelRatioF();
        m_screenGeometry.setWidth(m_screenGeometry.width() / ratio);
        m_screenGeometry.setHeight(m_screenGeometry.height() / ratio);
    }
    move(m_screenGeometry.center() - rect().center());
}

Manager::Manager()
{
    setupDialogs();
}

void Manager::setupDialogs()
{
    org::deepin::dde::Display1 *displayInter = new org::deepin::dde::Display1("org.deepin.dde.Display1", "/org/deepin/dde/Display1", QDBusConnection::sessionBus(), this);
    QList<QDBusObjectPath> screenList = displayInter->monitors();
    for (int i = 0; i < screenList.length(); i++) {
        MonitorInter *monitor = new MonitorInter("org.deepin.dde.Display1", screenList[i].path(), QDBusConnection::sessionBus());
        if (monitor->name().toLower().contains("edp"))
            continue;

        const QRect geom = QRect(monitor->x(), monitor->y(), monitor->width(), monitor->height());
        SuspendDialog *dialog = new SuspendDialog(geom);
        dialog->show();

        connect(dialog, &SuspendDialog::finished, this, &Manager::finished);
    }
}
