/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "xscreensaverbasedpoller.h"

#include <config-kidletime.h>

#include <QGuiApplication>

// sourcing the moc file before the Xlib include,
// whose defines like "Bool" break Qt names
#include "moc_xscreensaverbasedpoller.cpp"

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

static Display *display()
{
    return qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display();
}

XScreensaverBasedPoller::XScreensaverBasedPoller(QObject *parent)
    : KWindowBasedIdleTimePoller(parent)
    , m_screenSaverIface(nullptr)
{
}

XScreensaverBasedPoller::~XScreensaverBasedPoller()
{
}

bool XScreensaverBasedPoller::additionalSetUp()
{
    m_screenSaverIface = new OrgFreedesktopScreenSaverInterface(QLatin1String("org.freedesktop.ScreenSaver"),
                                                                QLatin1String("/ScreenSaver"),
                                                                QDBusConnection::sessionBus(),
                                                                this);

    connect(m_screenSaverIface, &OrgFreedesktopScreenSaverInterface::ActiveChanged, this, &XScreensaverBasedPoller::screensaverActivated);

    return true;
}

void XScreensaverBasedPoller::screensaverActivated(bool activated)
{
    // We care only if it has been disactivated

    if (!activated) {
        m_screenSaverIface->SimulateUserActivity();
        Q_EMIT resumingFromIdle();
    }
}

int XScreensaverBasedPoller::getIdleTime()
{
    XScreenSaverInfo *mitInfo = nullptr;
    mitInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(display(), DefaultRootWindow(display()), mitInfo);
    int ret = mitInfo->idle;
    XFree(mitInfo);
    return ret;
}

void XScreensaverBasedPoller::simulateUserActivity()
{
    stopCatchingIdleEvents();
    XResetScreenSaver(display());
    XFlush(display());
}
