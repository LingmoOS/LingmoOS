/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "activity.h"

#include <QFile>
#include <QCursor>
#include <QDebug>
#include <QX11Info>
#include <QDirIterator>
#include <QSettings>
#include <QRegularExpression>

#include <NETWM>
#include <KWindowSystem>

static const QStringList blockList = {"lingmo-launcher",
                                      "lingmo-statusbar"};

Activity::Activity(QObject *parent)
    : QObject(parent)
    , m_cApps(CApplications::self())
{
    onActiveWindowChanged();

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &Activity::onActiveWindowChanged);
    connect(KWindowSystem::self(), static_cast<void (KWindowSystem::*)(WId id, NET::Properties properties, NET::Properties2 properties2)>(&KWindowSystem::windowChanged),
            this, &Activity::onActiveWindowChanged);
}

bool Activity::launchPad() const
{
    return m_launchPad;
}

QString Activity::title() const
{
    return m_title;
}

QString Activity::icon() const
{
    return m_icon;
}

void Activity::close()
{
    NETRootInfo(QX11Info::connection(), NET::CloseWindow).closeWindowRequest(KWindowSystem::activeWindow());
}

void Activity::minimize()
{
    KWindowSystem::minimizeWindow(KWindowSystem::activeWindow());
}

void Activity::restore()
{
    KWindowSystem::clearState(KWindowSystem::activeWindow(), NET::Max);
}

void Activity::maximize()
{
    KWindowSystem::setState(KWindowSystem::activeWindow(), NET::Max);
}

void Activity::toggleMaximize()
{
    KWindowInfo info(KWindowSystem::activeWindow(), NET::WMState);
    bool isWindow = !info.hasState(NET::SkipTaskbar) ||
                     info.windowType(NET::UtilityMask) != NET::Utility ||
                     info.windowType(NET::DesktopMask) != NET::Desktop;

    if (!isWindow)
        return;

    bool isMaximized = info.hasState(NET::Max);
    isMaximized ? restore() : maximize();
}

void Activity::move()
{
    WId winId = KWindowSystem::activeWindow();
    KWindowInfo info(winId, NET::WMState | NET::WMGeometry | NET::WMDesktop);
    bool window = !info.hasState(NET::SkipTaskbar) ||
                     info.windowType(NET::UtilityMask) != NET::Utility ||
                     info.windowType(NET::DesktopMask) != NET::Desktop;

    if (!window)
        return;

    bool onCurrent = info.isOnCurrentDesktop();
    if (!onCurrent) {
        KWindowSystem::setCurrentDesktop(info.desktop());
        KWindowSystem::forceActiveWindow(winId);
    }

    NETRootInfo ri(QX11Info::connection(), NET::WMMoveResize);
    ri.moveResizeRequest(winId,
                         QCursor::pos().x(),
                         QCursor::pos().y(), NET::Move);
}

bool Activity::isAcceptableWindow(quint64 wid)
{
    QFlags<NET::WindowTypeMask> ignoreList;
    ignoreList |= NET::DesktopMask;
    ignoreList |= NET::DockMask;
    ignoreList |= NET::SplashMask;
    ignoreList |= NET::ToolbarMask;
    ignoreList |= NET::MenuMask;
    ignoreList |= NET::PopupMenuMask;
    ignoreList |= NET::NotificationMask;

    KWindowInfo info(wid, NET::WMWindowType | NET::WMState, NET::WM2TransientFor | NET::WM2WindowClass);

    if (!info.valid())
        return false;

    if (NET::typeMatchesMask(info.windowType(NET::AllTypesMask), ignoreList))
        return false;

    if (info.hasState(NET::SkipTaskbar) || info.hasState(NET::SkipPager))
        return false;

    // WM_TRANSIENT_FOR hint not set - normal window
    WId transFor = info.transientFor();
    if (transFor == 0 || transFor == wid || transFor == (WId) QX11Info::appRootWindow())
        return true;

    info = KWindowInfo(transFor, NET::WMWindowType);

    QFlags<NET::WindowTypeMask> normalFlag;
    normalFlag |= NET::NormalMask;
    normalFlag |= NET::DialogMask;
    normalFlag |= NET::UtilityMask;

    return !NET::typeMatchesMask(info.windowType(NET::AllTypesMask), normalFlag);
}

void Activity::onActiveWindowChanged()
{
    KWindowInfo info(KWindowSystem::activeWindow(),
                     NET::WMState | NET::WMVisibleName | NET::WMWindowType,
                     NET::WM2WindowClass);

    m_launchPad = (info.windowClassClass() == "lingmo-launcher");
    emit launchPadChanged();

    if (NET::typeMatchesMask(info.windowType(NET::AllTypesMask), NET::DesktopMask)) {
        m_title = tr("");
        m_icon = "sysytemlogo";

        emit titleChanged();
        emit iconChanged();

        return;
    }

    if (!isAcceptableWindow(KWindowSystem::activeWindow())
            || blockList.contains(info.windowClassClass())) {
        clearTitle();
        clearIcon();
        return;
    }

    m_pid = info.pid();
    m_windowClass = info.windowClassClass().toLower();

    CAppItem *item = m_cApps->matchItem(m_pid, m_windowClass);

    if (item) {
        m_title = item->localName;
        emit titleChanged();

        if (m_icon != item->icon) {
            m_icon = item->icon;
            emit iconChanged();
        }

    } else {
        QString title = info.visibleName();
        if (title != m_title) {
            m_title = title;
            emit titleChanged();
            clearIcon();
        }
    }
}

void Activity::clearTitle()
{
    m_title.clear();
    emit titleChanged();
}

void Activity::clearIcon()
{
    m_icon.clear();
    emit iconChanged();
}
