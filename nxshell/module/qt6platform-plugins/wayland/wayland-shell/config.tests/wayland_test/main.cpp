// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <private/qguiapplication_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>
#include <qpa/qplatformcursor.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qwindowsysteminterface.h>
#include <private/qxkbcommon_p.h>
#ifndef D_LINGMO_IS_DWAYLAND
#include <KWayland/Client/registry.h>
#include <KWayland/Client/oceanshell.h>
#else
#include <DWayland/Client/registry.h>
#include <DWayland/Client/oceanshell.h>
#endif


using namespace QtWaylandClient;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QWaylandDisplay *display =
        static_cast<QWaylandIntegration *>(
            QGuiApplicationPrivate::platformIntegration())
            ->display();
    display->usingInputContextFromCompositor();
#endif
    return 0;
}
