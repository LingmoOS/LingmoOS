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
#include <unistd.h>

#include "abstractinterface.h"
#include <QGuiApplication>
#include <QRect>
#include <QScreen>

using namespace kdk;

AbstractInterface::AbstractInterface(QObject *parent)
    :QObject(parent)
{
}

AbstractInterface::~AbstractInterface()
{
}

bool AbstractInterface::inCurrentDesktopActivity(const WindowInfo &winfo)
{
    if(winfo.isValid() && winfo.isOnDesktop(currentDesktop()))
    return (winfo.isValid() && winfo.isOnDesktop(currentDesktop()) /*&& winfo.isOnActivity(currentActivity())*/);
}

bool AbstractInterface::isPlasmaDesktop(const QRect &wGeometry)
{
    if (wGeometry.isEmpty()) {
        return false;
    }

    for (const auto scr : qGuiApp->screens()) {
        if (wGeometry == scr->geometry()) {
            return true;
        }
    }

    return false;
}

QString AbstractInterface::currentDesktop()
{
    return m_currentDesktop;
}

QString AbstractInterface::currentActivity()
{
    return m_currentActivity;
}

void AbstractInterface::setPlasmaDesktop(WindowId wid)
{
    if (!m_windows.contains(wid)) {
        return;
    }

    if (!m_windows[wid].isPlasmaDesktop()) {
        m_windows[wid].setIsPlasmaDesktop(true);
        //updateAllHints();
    }
}

bool AbstractInterface::isValidFor(const WindowId &wid)
{
    if (!m_windows.contains(wid)) {
        return false;
    }

    return m_windows[wid].isValid() && !m_windows[wid].isPlasmaDesktop();
}

QList<WindowId> AbstractInterface::windows()
{
    return m_windows.keys();
}

QString AbstractInterface::getProcessName(const WindowId &wid)
{
    auto id = pid(wid);
    char path[128];
    memset(path,0,128);
    char name[128];
    memset(name,0,128);
    if(!name)
    {
        return NULL;
    }
    char link[512];
    memset(link,0,512);
    sprintf(path, "/proc/%d/exe", id);

    int count = readlink(path, link, sizeof(link));
    if(count != -1)
    {
        strcpy(name, basename(link));
    }
    return QString(name);
}

bool AbstractInterface::setWindowRadius(QWindow *windowHandle, int radius)
{
    Q_UNUSED(windowHandle)
    Q_UNUSED(radius)
    return false;
}

void AbstractInterface::activateWindow(QWindow *window1, QWindow *window2)
{
    Q_UNUSED(window1)
    Q_UNUSED(window2)
    return;
}

void AbstractInterface::setIdleInterval(int msec)
{
    Q_UNUSED(msec)
    return;
}
QRect AbstractInterface::windowGeometry(const WindowId& windowId)
{
    Q_UNUSED(windowId)
    return QRect();
}

void AbstractInterface::setPanelAutoHide(QWindow *window, bool autoHide)
{
    Q_UNUSED(window)
    Q_UNUSED(autoHide)
    return;
}

void AbstractInterface::setGrabKeyboard(QWindow *window, bool grabKeyboard)
{
    Q_UNUSED(window)
    Q_UNUSED(grabKeyboard)
    return;
}

void AbstractInterface::setWindowLayer(QWindow *window, WindowLayer layer)
{
    Q_UNUSED(window)
    Q_UNUSED(layer)
    return;
}

WindowLayer AbstractInterface::windowLayer(QWindow *window)
{
    Q_UNUSED(window)
    return WindowLayer::Normal;
}

void AbstractInterface::setHighlight(const WindowId &wid, bool highlight)
{
    Q_UNUSED(wid)
    Q_UNUSED(highlight)
}

bool AbstractInterface::istHighlight(const WindowId &wid)
{
    Q_UNUSED(wid)
}

void AbstractInterface::setOpenUnderCursor(QWindow *window)
{
    Q_UNUSED(window)
}

void AbstractInterface::setOpenUnderCursor(QWindow *window, int x, int y)
{
    Q_UNUSED(window)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void AbstractInterface::setIconName(QWindow *window, const QString& iconName)
{
    Q_UNUSED(window)
    Q_UNUSED(iconName)
}

QString AbstractInterface::currentSeatName()
{
    return QString();
}

QString AbstractInterface::currentOutputName()
{
    return QString();
}

QList<OutputInfo *> AbstractInterface::outputsInfo()
{
    QList<OutputInfo *> list;
    return list;
}

QList<WindowId> AbstractInterface::getWindowIdByPid(quint32 pid)
{
    Q_UNUSED(pid)
    QList<WindowId> list;
    return list;
}

QList<WindowId> AbstractInterface::getWindowIdByTtile(const QString &title)
{
    Q_UNUSED(title)
    QList<WindowId> list;
    return list;
}
