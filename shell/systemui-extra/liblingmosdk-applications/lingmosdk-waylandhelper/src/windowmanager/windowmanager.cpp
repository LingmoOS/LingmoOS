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

#include "windowmanager.h"
#include <NETWM>
#include <QDebug>
#include <kwindowinfo.h>

using namespace kdk;

static WindowManager* g_windowmanger = nullptr;
static WmRegister* m_wm =nullptr;
kdk::WindowManager::WindowManager(QObject *parent)
    :QObject(parent)
{
    m_wm = new WmRegister(this);

    connect(m_wm->winInterface(),&AbstractInterface::windowAdded,this,&WindowManager::windowAdded);
    connect(m_wm->winInterface(),&AbstractInterface::windowRemoved,this,&WindowManager::windowRemoved);
    connect(m_wm->winInterface(),&AbstractInterface::activeWindowChanged,this,&WindowManager::activeWindowChanged);
    connect(m_wm->winInterface(),&AbstractInterface::windowChanged,this,&WindowManager::windowChanged);
    connect(m_wm->winInterface(),&AbstractInterface::currentDesktopChanged,this,&WindowManager::currentDesktopChanged);
    connect(m_wm->winInterface(),&AbstractInterface::isShowingDesktopChanged,this,&WindowManager::isShowingDesktopChanged);

    connect(m_wm->winInterface(),&AbstractInterface::titleChanged,this,&WindowManager::titleChanged);
    connect(m_wm->winInterface(),&AbstractInterface::iconChanged,this,&WindowManager::iconChanged);
    connect(m_wm->winInterface(),&AbstractInterface::activeChanged,this,&WindowManager::activeChanged);
    connect(m_wm->winInterface(),&AbstractInterface::fullscreenChanged,this,&WindowManager::fullscreenChanged);
    connect(m_wm->winInterface(),&AbstractInterface::keepAboveChanged,this,&WindowManager::keepAboveChanged);
    connect(m_wm->winInterface(),&AbstractInterface::minimizedChanged,this,&WindowManager::minimizedChanged);
    connect(m_wm->winInterface(),&AbstractInterface::maximizedChanged,this,&WindowManager::maximizedChanged);
    connect(m_wm->winInterface(),&AbstractInterface::onAllDesktopsChanged,this,&WindowManager::onAllDesktopsChanged);
    connect(m_wm->winInterface(),&AbstractInterface::demandsAttentionChanged,this,&WindowManager::demandsAttentionChanged);
    connect(m_wm->winInterface(),&AbstractInterface::skipTaskbarChanged,this,&WindowManager::skipTaskbarChanged);
    connect(m_wm->winInterface(),&AbstractInterface::skipSwitcherChanged,this,&WindowManager::skipSwitcherChanged);
    connect(m_wm->winInterface(),&AbstractInterface::geometryChanged,this,&WindowManager::geometryChanged);
    connect(m_wm->winInterface(),&AbstractInterface::outputInfoChanged,this,&WindowManager::outputInfoChanged);
}

WindowId WindowManager::currentActiveWindow()
{
    self();
    if(!m_wm)
        return QVariant();
    return m_wm->winInterface()->activeWindow();
}

void WindowManager::keepWindowAbove(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->requestToggleKeepAbove(windowId);
}

QString WindowManager::getWindowTitle(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return QString();
    return m_wm->winInterface()->titleFor(windowId);
}

QIcon WindowManager::getWindowIcon(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return QIcon();
    return m_wm->winInterface()->iconFor(windowId);
}

QString WindowManager::getWindowGroup(const WindowId &windowId)
{
    if(!m_wm)
        return QString();
    self();
    return m_wm->winInterface()->windowGroupFor(windowId);
}

void WindowManager::closeWindow(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->requestClose(windowId);
}

void WindowManager::activateWindow(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->requestActivate(windowId);
}

void WindowManager::maximizeWindow(const WindowId &windowId)
{
     self();
    if(!m_wm)
        return;
    m_wm->winInterface()->requestToggleMaximized(windowId);
}

void WindowManager::minimizeWindow(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->requestToggleMinimized(windowId);
}

quint32 WindowManager::getPid(const WindowId &windowId)
{
    self();
    quint32 pid = 0;
    if(!m_wm)
        return pid;
    pid = m_wm->winInterface()->pid(windowId);
    return pid;
}

WindowManager *WindowManager::self()
{
    if(g_windowmanger)
        return g_windowmanger;
    g_windowmanger = new WindowManager();
    return g_windowmanger;
}

WindowInfo WindowManager::getwindowInfo(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return WindowInfo();
    return m_wm->winInterface()->requestInfo(windowId);
}

void WindowManager::showDesktop()
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->showCurrentDesktop();
}

void WindowManager::hideDesktop()
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->hideCurrentDesktop();
}

QString WindowManager::currentDesktop()
{
    self();
    if(!m_wm)
        return 0;
    return m_wm->winInterface()->currentDesktop();
}

QList<WindowId> WindowManager::windows()
{
    self();
    if(!m_wm)
        return QList<WindowId>();
    return m_wm->winInterface()->windows();
}

NET::WindowType WindowManager::getWindowType(const WindowId &windowId)
{
     self();
    if(!m_wm)
        return NET::WindowType::Unknown;
    return m_wm->winInterface()->windowType(windowId);
}

void WindowManager::setGeometry(QWindow *window, const QRect &rect)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->setGeometry(window,rect);
}

void WindowManager::setSkipTaskBar(QWindow *window, bool skip)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->setSkipTaskBar(window,skip);
}

void WindowManager::setSkipSwitcher(QWindow *window, bool skip)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->setSkipSwitcher(window,skip);
}

bool WindowManager::skipTaskBar(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return false;
    return m_wm->winInterface()->skipTaskBar(windowId);
}

bool WindowManager::skipSwitcher(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return false;
    return m_wm->winInterface()->skipSwitcher(windowId);
}

bool WindowManager::isShowingDesktop()
{
    self();
    if(!m_wm)
        return false;
    return m_wm->winInterface()->isShowingDesktop();
}

void WindowManager::setOnAllDesktops(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->setOnAllDesktops(windowId);
}

bool WindowManager::isOnAllDesktops(const WindowId &windowId)
{
    kdk::WindowInfo windowInfo =  WindowManager::getwindowInfo(windowId);
    return windowInfo.isOnAllDesktops();
}

bool WindowManager::isOnCurrentDesktop(const WindowId &windowId)
{
    kdk::WindowInfo  windowinfo = WindowManager::getwindowInfo(windowId);
    return windowinfo.isOnDesktop(currentDesktop());
}

bool WindowManager::isOnDesktop(const WindowId &windowId, int desktop)
{
   kdk::WindowInfo  windowinfo = WindowManager::getwindowInfo(windowId);
   return windowinfo.isOnDesktop(QString::number(desktop));
}

void WindowManager::setPanelTakefocus(QWindow *window,bool flag)
{
    self();
    if(!m_wm || !window)
        return;
    m_wm->winInterface()->setPanelTakefocus(window,flag);
}

void WindowManager::demandAttention(const WindowId &wid)
{
    self();
    if(!m_wm)
        return;
    m_wm->winInterface()->demandAttention(wid);
}

QString WindowManager::getProcessName(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return QString();
    return m_wm->winInterface()->getProcessName(windowId);
}

void WindowManager::setWindowRadius(QWindow *window, int radius)
{
    self();
    if(!m_wm)
        return ;
    m_wm->winInterface()->setWindowRadius(window,radius);
}

void WindowManager::activateWindow(QWindow *window1, QWindow *window2)
{
    self();
    if(!m_wm)
        return ;
    m_wm->winInterface()->activateWindow(window1,window2);
}

QRect WindowManager::windowGeometry(const WindowId &windowId)
{
    self();
    if(!m_wm)
        return QRect();
    return m_wm->winInterface()->windowGeometry(windowId);
}

void WindowManager::setPanelAutoHide(QWindow *window, bool autoHide)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setPanelAutoHide(window, autoHide);
}

void WindowManager::setGrabKeyboard(QWindow *window, bool grabKeyboard)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setGrabKeyboard(window, grabKeyboard);
}

void WindowManager::setWindowLayer(QWindow *window, WindowLayer layer)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setWindowLayer(window, layer);
}

WindowLayer WindowManager::windowLayer(QWindow *window)
{
    self();
    if(!m_wm)
        return WindowLayer::Normal;
    return m_wm->winInterface()->windowLayer(window);
}

void WindowManager::setHighlight(const WindowId &wid, bool highlight)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setHighlight(wid, highlight);
}

bool WindowManager::isHightlight(const WindowId &wid)
{
    self();
    if(!m_wm)
        return false;
    return m_wm->winInterface()->istHighlight(wid);
}

void WindowManager::setOpenUnderCursor(QWindow *window)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setOpenUnderCursor(window);
}

void WindowManager::setOpenUnderCursor(QWindow *window, int x, int y)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setOpenUnderCursor(window, x, y);
}

void WindowManager::setIconName(QWindow *window, const QString &iconName)
{
    self();
    if(!m_wm)
        return ;
    return m_wm->winInterface()->setIconName(window, iconName);
}

QString WindowManager::currentSeatName()
{
    self();
    if(!m_wm)
        return QString();
    return m_wm->winInterface()->currentSeatName();
}

QString WindowManager::currentOutputName()
{
    self();
    if(!m_wm)
        return QString();
    return m_wm->winInterface()->currentOutputName();
}

QList<OutputInfo *> WindowManager::outputsInfo()
{
    QList<OutputInfo *> list;
    self();
    if(!m_wm)
        return list;
    return m_wm->winInterface()->outputsInfo();
}

QList<WindowId> WindowManager::getWindowIdByPid(quint32 pid)
{
    QList<WindowId> list;
    self();
    if(!m_wm)
        return list;
    return m_wm->winInterface()->getWindowIdByPid(pid);
}

QList<WindowId> WindowManager::getWindowIdByTtile(const QString&title)
{
    QList<WindowId> list;
    self();
    if(!m_wm)
        return list;
    return m_wm->winInterface()->getWindowIdByTtile(title);
}
