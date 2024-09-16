// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "portalwaylandcontext.h"
#include "screenshotportal.h"

#include <QGuiApplication>
#include <qpa/qplatformintegration.h>
#include <private/qwaylandintegration_p.h>
#include <private/qguiapplication_p.h>
#include <QTimer>

using namespace QtWaylandClient;

PortalWaylandContext::PortalWaylandContext(QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , m_screenCopyManager(new ScreenCopyManager(this))
{
    auto screenShotPortal = new ScreenshotPortalWayland(this);
}
