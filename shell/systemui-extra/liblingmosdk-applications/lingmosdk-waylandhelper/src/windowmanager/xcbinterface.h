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

#ifndef XCBINTERFACE_H
#define XCBINTERFACE_H

#include "abstractinterface.h"

#include <KWindowInfo>
#include <KWindowEffects>

using namespace kdk;
class XcbInterface:public AbstractInterface
{
public:
    XcbInterface(QObject* parent=nullptr);
    ~XcbInterface();
    WindowInfo requestInfo(WindowId wid) override;

    void requestActivate(WindowId wid)  override;
    void requestClose(WindowId wid) override;
    void requestToggleKeepAbove(WindowId wid)  override;
    void requestToggleMinimized(WindowId wid)  override;
    void requestToggleMaximized(WindowId wid)  override;

    QIcon iconFor(WindowId wid) override;
    QString titleFor(WindowId wid) override;
    QString windowGroupFor(WindowId wid) override;

    void showCurrentDesktop() override;
    void hideCurrentDesktop() override;

    quint32 pid(WindowId wid) override;

    WindowId activeWindow() override;

    bool windowCanBeDragged(WindowId wid) override;
    bool windowCanBeMaximized(WindowId wid) override;

    void setGeometry(QWindow *window, const QRect &rect) override;
    void setSkipTaskBar(QWindow* window,bool skip) override;
    void setSkipSwitcher(QWindow* window,bool skip) override;

    bool skipTaskBar(const WindowId &wid) override;
    bool skipSwitcher(const WindowId &wid) override;

    bool isShowingDesktop() override;
    void setOnAllDesktops(const WindowId &wid) override;

    NET::WindowType windowType(WindowId wid) override;
    void setPanelTakefocus(QWindow *window, bool flag) override;
    void demandAttention(const WindowId &wid) override;
    QRect windowGeometry(const WindowId& windowId) override;

    void setWindowLayer(QWindow *window, WindowLayer layer) override;
    WindowLayer windowLayer(QWindow *window) override;

    QList<WindowId> getWindowIdByPid(quint32 pid) override;
    QList<WindowId> getWindowIdByTtile(const QString& title) override;

private:
    bool isValidWindow(WindowId wid) ;
    bool isValidWindow(const KWindowInfo &winfo) ;
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);

private:
    WindowId m_desktopId{-1};
};

#endif // XCBINTERFACE_H
