// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWaylandShellManager_H
#define DWaylandShellManager_H

#include "vtablehook.h"

#define private public
#include "QtWaylandClient/private/qwaylandintegration_p.h"
#include "QtWaylandClient/private/qwaylandshellintegrationplugin_p.h"
#include "QtWaylandClient/private/qwaylandshellintegration_p.h"
#include "QtWaylandClient/private/qwaylandshellsurface_p.h"
#include "QtWaylandClient/private/qwaylandwindow_p.h"
#include "QtWaylandClient/private/qwaylandcursor_p.h"
#include "QtWaylandClient/private/qwaylandscreen_p.h"
#undef private

#ifndef D_DEEPIN_IS_DWAYLAND
#include <KWayland/Client/registry.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/server_decoration.h>
#include <KWayland/Client/ddeshell.h>
#include <KWayland/Client/ddeseat.h>
#include <KWayland/Client/ddekeyboard.h>
#include <KWayland/Client/strut.h>
#include <KWayland/Client/fakeinput.h>
#include <KWayland/Client/compositor.h>
#include <KWayland/Client/blur.h>
#include <KWayland/Client/region.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#else
#include <DWayland/Client/registry.h>
#include <DWayland/Client/plasmashell.h>
#include <DWayland/Client/server_decoration.h>
#include <DWayland/Client/ddeshell.h>
#include <DWayland/Client/ddeseat.h>
#include <DWayland/Client/ddekeyboard.h>
#include <DWayland/Client/strut.h>
#include <DWayland/Client/fakeinput.h>
#include <DWayland/Client/compositor.h>
#include <DWayland/Client/blur.h>
#include <DWayland/Client/region.h>
#include <DWayland/Client/surface.h>
#include <DWayland/Client/plasmawindowmanagement.h>
#endif

#include <QGuiApplication>
#include <QPointer>
#include <QDebug>
#include <qpa/qplatformnativeinterface.h>
#include <private/qguiapplication_p.h>
#include <private/qwidgetwindow_p.h>

DPP_USE_NAMESPACE
using namespace KWayland::Client;

namespace QtWaylandClient {

class DWaylandShellManager
{
    DWaylandShellManager();
    ~DWaylandShellManager();
    Registry *m_registry = nullptr;
public:
    static DWaylandShellManager *instance() {
        static DWaylandShellManager manager;
        return &manager;
    }
    static Registry *registry() {
        return instance()->m_registry;
    }

    static void sendProperty(QWaylandShellSurface *self, const QString &name, const QVariant &value);
    static void requestActivateWindow(QPlatformWindow *self);
    static bool disableClientDecorations(QWaylandShellSurface *surface);
    static QMargins frameMargins(QPlatformWindow *self);
    static void setWindowFlags(QPlatformWindow *self, Qt::WindowFlags flags);
    static void createServerDecoration(QWaylandShellSurface *shellSurface);
    static void setGeometry(QPlatformWindow *self, const QRect &rect);
    static void pointerEvent(const QPointF &pointF, QEvent::Type type);
    static QWaylandShellSurface *createShellSurface(QWaylandShellIntegration *self, QWaylandWindow *window);
    static void createKWaylandShell(quint32 name, quint32 version);
    static void createKWaylandSSD(quint32 name, quint32 version);
    static void createDDEShell(quint32 name, quint32 version);
    static void createDDESeat(quint32 name, quint32 version);
    static void createStrut(quint32 name, quint32 version);
    static void createDDEPointer();
    static void createDDEKeyboard();
    static void createDDEFakeInput();
    static void createBlurManager(quint32 name, quint32 version);
    static void createCompositor(quint32 name, quint32 version);
    static void createSurface();
    static void createPlasmaWindowManagement(KWayland::Client::Registry *registry, quint32 name, quint32 version);
    static void handleGeometryChange(QWaylandShellSurface *shellSurface);
    static void handleWindowStateChanged(QWaylandShellSurface *shellSurface);
    static void setWindowStaysOnTop(QWaylandShellSurface *surface, const bool state);
    static void setDockStrut(QWaylandShellSurface *surface, const QVariant var);
    static void setCursorPoint(QPointF pos);
    static void setEnableBlurWidow(QWaylandWindow *wlWindow, const QVariant &value);
    static void updateWindowBlurAreasForWM(QWaylandWindow *wlWindow, const QString &name, const QVariant &value);
    static void setDockAppItemMinimizedGeometry(QWaylandShellSurface *surface, const QVariant var);

    static void onShellSurfaceCreated(QWaylandShellSurface *shellSurface);
private:
    // 用于记录设置过以_DWAYALND_开头的属性，当kwyalnd_shell对象创建以后要使这些属性生效
    static QList<QPointer<QWaylandWindow>> send_property_window_list;
};
}

#endif // DWaylandShellManager_H
