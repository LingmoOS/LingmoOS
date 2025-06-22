/*
    SPDX-FileCopyrightText: 2015 Bhushan Shah <bhush94@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "waylandlocker.h"

#include <QGuiApplication>
#include <QScreen>

namespace ScreenLocker
{
WaylandLocker::WaylandLocker(QObject *parent)
    : AbstractLocker(parent)
{
    if (m_background) {
        updateGeometryOfBackground();
        const auto screens = qApp->screens();
        for (auto s : screens) {
            connect(s, &QScreen::geometryChanged, this, &WaylandLocker::updateGeometryOfBackground);
        }
        connect(qApp, &QGuiApplication::screenAdded, this, [this](QScreen *s) {
            connect(s, &QScreen::geometryChanged, this, &WaylandLocker::updateGeometryOfBackground);
            updateGeometryOfBackground();
        });
        connect(qApp, &QGuiApplication::screenRemoved, this, &WaylandLocker::updateGeometryOfBackground);
    }
}

WaylandLocker::~WaylandLocker()
{
}

void WaylandLocker::updateGeometryOfBackground()
{
    QRect combined;
    const auto screens = qApp->screens();
    for (auto s : screens) {
        combined = combined.united(s->geometry());
    }
    m_background->setGeometry(combined);
    m_background->update();
}

void WaylandLocker::showLockWindow()
{
}

void WaylandLocker::hideLockWindow()
{
}

void WaylandLocker::addAllowedWindow(quint32 window)
{
    Q_UNUSED(window)
    Q_EMIT lockWindowShown();
}

void WaylandLocker::stayOnTop()
{
}

}
