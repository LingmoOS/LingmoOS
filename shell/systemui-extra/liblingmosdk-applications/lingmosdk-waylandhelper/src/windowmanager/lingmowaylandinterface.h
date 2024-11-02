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

#ifndef LINGMOWAYLANDINTERFACE_H
#define LINGMOWAYLANDINTERFACE_H

#include <QObject>
#include <QMap>

#include "lingmoshell.h"
#include "lingmowindowmanagement.h"
#include "xdgactivation.h"
#include "extidlenotifier.h"

#include "abstractinterface.h"
#include "kdewaylandinterface.h"

class LingmoUIWaylandInterface : public AbstractInterface
{
    Q_OBJECT
public:
    explicit LingmoUIWaylandInterface(QObject *parent = nullptr);
    ~LingmoUIWaylandInterface();

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

    bool lingmoProtocolReady();
    Registry* registry();
    void handleGlobal(void *data, struct wl_registry *registry,
                              uint32_t name, const char *interface, uint32_t version);
    void handleGlobalRemove(void *data, struct wl_registry *registry,
                            uint32_t name);

    bool removeHeaderBar(QWindow *windowHandle);
    bool setWindowRadius(QWindow *windowHandle,int radius) override;
    void activateWindow(QWindow*window1,QWindow*window2) override;
    void setIdleInterval(int msec) override;
    QRect windowGeometry(const WindowId& windowId) override;
    void setPanelAutoHide(QWindow *window, bool autoHide) override;
    void setGrabKeyboard(QWindow *window, bool autoHide) override;

    void setWindowLayer(QWindow *window, WindowLayer layer) override;
    WindowLayer windowLayer(QWindow *window) override;

    void setHighlight(const WindowId &wid, bool highlight) override;
    bool istHighlight(const WindowId &wid) override;

    void setOpenUnderCursor(QWindow *window) override;
    void setOpenUnderCursor(QWindow *window, int x, int y) override;
    void setIconName(QWindow *window, const QString& iconName) override;

    QString currentSeatName() override;
    QString currentOutputName() override;
    QList<OutputInfo*> outputsInfo() override;

    QList<WindowId> getWindowIdByPid(quint32 pid) override;
    QList<WindowId> getWindowIdByTtile(const QString& title) override;

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    LingmoUIWindow* windowFor(WindowId wid);
    void lingmoWindowCreatedProxy(LingmoUIWindow* w);
    void trackLingmoUIWindow(LingmoUIWindow *w);
    void untrackLingmoUIWindow(LingmoUIWindow *w);
    static const struct wl_registry_listener s_registryListener;

private:
    ExtIdleNotifier *m_notifier = nullptr;
    wl_seat *m_seat = nullptr;
    Shell *m_shell = nullptr;
    ConnectionThread *m_connection = nullptr;
    Registry* m_registry = nullptr;
    QMap<QWindow*,Surface*>m_surfaces;
    QStringList m_desktops;
    LingmoUIShell *m_lingmoShell = nullptr;
    LingmoUIWindowManagement *m_lingmoWindowManager = nullptr;
    XdgActivation *m_xdgActivation = nullptr;
    XdgActivationToken * m_xdgActivationToken = nullptr;
    QMap<QWindow*,LingmoUIShellSurface*>m_lingmoShellSurfaces;
};

#endif // LINGMOWAYLANDINTERFACE_H
