// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef USE_DEEPIN_WAYLAND
#include "QtWaylandClient/private/qwaylandintegration_p.h"
#include "QtWaylandClient/private/qwaylandshellintegrationplugin_p.h"
namespace QtWaylandClient {
class QKWaylandShellIntegrationPlugin : public QWaylandShellIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandShellIntegrationFactoryInterface_iid FILE "kwayland-shell.json")

public:
    QWaylandShellIntegration *create(const QString &key, const QStringList &paramList) override {
        Q_UNUSED(key);
        Q_UNUSED(paramList);
        return nullptr;
    }
};
}

#else

#include "vtablehook.h"
#include "dwaylandshellmanager.h"

#define private public
#include "QtWaylandClient/private/qwaylandintegration_p.h"
#include "QtWaylandClient/private/qwaylandshellintegrationplugin_p.h"
#include "QtWaylandClient/private/qwaylandshellintegration_p.h"
#include "QtWaylandClient/private/qwaylandshellsurface_p.h"
#include "QtWaylandClient/private/qwaylandwindow_p.h"
#undef private

#ifndef D_DEEPIN_IS_DWAYLAND
#include <KWayland/Client/registry.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/server_decoration.h>
#include <KWayland/Client/ddeshell.h>
#else
#include <DWayland/Client/registry.h>
#include <DWayland/Client/plasmashell.h>
#include <DWayland/Client/server_decoration.h>
#include <DWayland/Client/ddeshell.h>
#endif

#include <QGuiApplication>
#include <QPointer>
#include <QDebug>
#include <qpa/qplatformnativeinterface.h>
#include <private/qguiapplication_p.h>
#include <private/qwidgetwindow_p.h>

// 用于窗口设置和dwayland相关的特殊属性的前缀
// 以_d_dwayland_开头的属性需要做特殊处理，一般是用于和kwayland的交互
#define _DWAYALND_ "_d_dwayland_"

#define platformNativeDisplay \
    QGuiApplication::platformNativeInterface()-> \
    nativeResourceForIntegration(QByteArrayLiteral("display"))

DPP_USE_NAMESPACE

namespace QtWaylandClient {

class QKWaylandShellIntegrationPlugin : public QWaylandShellIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandShellIntegrationFactoryInterface_iid FILE "kwayland-shell.json")

public:
    QWaylandShellIntegration *create(const QString &key, const QStringList &paramList) override;
};

QWaylandShellIntegration *QKWaylandShellIntegrationPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(key)
    Q_UNUSED(paramList)

    Registry *registry = DWaylandShellManager::registry();

    connect(registry, &Registry::plasmaShellAnnounced,
            &DWaylandShellManager::createKWaylandShell);

    connect(registry, &Registry::serverSideDecorationManagerAnnounced,
            &DWaylandShellManager::createKWaylandSSD);

    //创建ddeshell
    connect(registry, &Registry::ddeShellAnnounced,
            &DWaylandShellManager::createDDEShell);

    //创建ddeseat
    connect(registry, &Registry::ddeSeatAnnounced,
            &DWaylandShellManager::createDDESeat);

    connect(registry, &Registry::interfacesAnnounced, [] {
        DWaylandShellManager::createDDEPointer();
        DWaylandShellManager::createDDEKeyboard();
        DWaylandShellManager::createDDEFakeInput();
    });

    connect(registry, &Registry::strutAnnounced,
            &DWaylandShellManager::createStrut);

    connect(registry, &Registry::blurAnnounced, [](quint32 name, quint32 version) {
        DWaylandShellManager::createBlurManager(name, version);
    });

    connect(registry, &Registry::compositorAnnounced, [](quint32 name, quint32 version){
        DWaylandShellManager::createCompositor(name, version);
        DWaylandShellManager::createSurface();
    });

    connect(registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced, [registry](quint32 name, quint32 version) {
        DWaylandShellManager::createPlasmaWindowManagement(registry, name, version);
    });

    wl_display *wlDisplay = reinterpret_cast<wl_display*>(platformNativeDisplay);

    registry->create(wlDisplay);
    registry->setup();

    wl_display_roundtrip(wlDisplay);

    auto wayland_integration = static_cast<QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
    QString shellVersion = registry->hasInterface(Registry::Interface::XdgShellUnstableV6) ? "xdg-shell-v6" : "xdg-shell";
    QWaylandShellIntegration *shell = wayland_integration->createShellIntegration(shellVersion);

    if (!shell) {
        qInfo() << "Failed to create kwayland-shell and the shell is nullptr.";
        return nullptr;
    }

    HookOverride(shell, &QWaylandShellIntegration::createShellSurface, DWaylandShellManager::createShellSurface);

    return shell;
}

}

#include "main.moc"
#endif
