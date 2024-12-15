// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shutdownapplet.h"
#include "treelandlockscreen.h"

#include <QDebug>
#include <QGuiApplication>

#include <DDBusSender>

#include <pluginfactory.h>
DCORE_USE_NAMESPACE

DS_BEGIN_NAMESPACE
namespace shutdown {

ShutdownApplet::ShutdownApplet(QObject *parent)
    : DApplet(parent)
{
}

ShutdownApplet::~ShutdownApplet()
{
}

bool ShutdownApplet::load()
{
    auto platformName = QGuiApplication::platformName();
    if (QStringLiteral("wayland") == platformName) {
        m_lockscreen.reset(new TreeLandLockScreen);
    }
    return true;
}

bool ShutdownApplet::requestShutdown()
{
    qDebug() << "request shutdown";
    if (m_lockscreen) {
        m_lockscreen->shutdown();
    } else {
        DDBusSender()
            .service("org.lingmo.ocean.ShutdownFront1")
            .interface("org.lingmo.ocean.ShutdownFront1")
            .path("/org/lingmo/ocean/ShutdownFront1")
            .method("Show")
            .call();
    }

    return true;
}

bool ShutdownApplet::requestLock()
{
    if (m_lockscreen) {
        m_lockscreen->lock();
    }
    return true;
}

bool ShutdownApplet::requestSwitchUser()
{
    if (m_lockscreen) {
        m_lockscreen->switchUser();
    }
    return true;
}

D_APPLET_CLASS(ShutdownApplet)
}
DS_END_NAMESPACE

#include "shutdownapplet.moc"
