/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KDEWAYLANDINTERFACE_H
#define KDEWAYLANDINTERFACE_H

#include <QObject>
#include <KWayland/Client/registry.h>
#include <KWayland/Server/plasmashell_interface.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmavirtualdesktop.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/shell.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/connection_thread.h>
#include "abstractinterface.h"

using namespace kdk;
using namespace KWayland::Client;

class  KdeWaylandInterface : public AbstractInterface
{
    Q_OBJECT
public:
    KdeWaylandInterface(QObject *parent = nullptr);
    ~KdeWaylandInterface();
    WindowInfo requestInfo(WindowId wid)override;

    void requestActivate(WindowId wid)override;
    void requestClose(WindowId wid)override;
    void requestToggleKeepAbove(WindowId wid)override;
    void requestToggleMinimized(WindowId wid)override;
    void requestToggleMaximized(WindowId wid)override;

    QIcon iconFor(WindowId wid)override;
    QString titleFor(WindowId wid)override;
    QString windowGroupFor(WindowId wid)override;
    quint32 pid(WindowId wid)override;

    void showCurrentDesktop()override;
    void hideCurrentDesktop()override;

    bool windowCanBeDragged(WindowId wid)override;
    bool windowCanBeMaximized(WindowId wid)override;

    WindowId activeWindow()override;

    void setGeometry(QWindow *window, const QRect &rect)override;

    void setSkipTaskBar(QWindow* window,bool skip)override;
    void setSkipSwitcher(QWindow* window,bool skip)override;

    bool skipTaskBar(const WindowId &wid)override;
    bool skipSwitcher(const WindowId &wid)override;

    bool isShowingDesktop() override;
    void setOnAllDesktops(const WindowId &wid)override;

    NET::WindowType windowType(WindowId wid)override;
    void setPanelTakefocus(QWindow *window, bool flag)override;
    void demandAttention(const WindowId &wid)override;
    QRect windowGeometry(const WindowId& windowId) override;

    void setWindowLayer(QWindow *window, WindowLayer layer) override;
    WindowLayer windowLayer(QWindow *window) override;


protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
    void updateWindow();
    void windowUnmapped();

private:
    PlasmaWindow *windowFor(WindowId wid) ;
    bool isValidWindow(const KWayland::Client::PlasmaWindow *w) ;
    bool isPlasmaDesktop(const KWayland::Client::PlasmaWindow *w) ;
    bool isPlasmaPanel(const KWayland::Client::PlasmaWindow *w) ;
    void windowCreatedProxy(KWayland::Client::PlasmaWindow *w);
    void trackWindow(KWayland::Client::PlasmaWindow *w);
    void untrackWindow(KWayland::Client::PlasmaWindow *w);
    void setCurrentDesktop(QString desktop);
    void addDesktop(const QString &id, quint32 position);
    void initKdeProtocol();

private:
    PlasmaShell *m_plasmaShell = nullptr;
    PlasmaWindowManagement *m_windowManager = nullptr;
    PlasmaVirtualDesktopManagement *m_virtualDesktopManagement = nullptr;
    Shell *m_shell = nullptr;
    ConnectionThread *m_connection = nullptr;
    Registry* m_registry = nullptr;
    QMap<QWindow*,Surface*>m_surfaces;
    QStringList m_desktops;
    QMap<QWindow*,PlasmaShellSurface*>m_plasmaShellSurfaces;

};

#endif // KDEWAYLANDINTERFACE_H
