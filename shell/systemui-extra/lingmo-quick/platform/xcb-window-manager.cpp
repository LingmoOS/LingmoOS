/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "xcb-window-manager.h"
#include <KWindowSystem>
#include <NETWM>
#include <QX11Info>
#include <QQuickWindow>
#include <QDebug>
namespace LingmoUIQuick {
XcbWindowManager::XcbWindowManager(QObject *parent): AbstractWindowManager(parent)
{
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &XcbWindowManager::windowAddedProxy);

    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, [=](WId wid){
        m_windowProperties.remove(wid);
        Q_EMIT windowRemoved(QString::number(wid));
    });
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &AbstractWindowManager::currentDesktopChanged);
    connect(KWindowSystem::self(), QOverload<WId, NET::Properties, NET::Properties2>::of(&KWindowSystem::windowChanged)
            , this, &XcbWindowManager::windowChangedProxy);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, [=](WId wid){
        Q_EMIT activeWindowChanged(QString::number(wid));
    });
}

XcbWindowManager::~XcbWindowManager()
{
}

QStringList XcbWindowManager::windows()
{
    QStringList windows;
    for(const WId &wid : KWindowSystem::windows()) {
        windows.append(QString::number(wid));
    }
    return windows;
}

QIcon XcbWindowManager::windowIcon(const QString &wid)
{
    QIcon icon;
    icon.addPixmap(KWindowSystem::icon(wid.toUInt(), 16, 16, false));
    icon.addPixmap(KWindowSystem::icon(wid.toUInt(), 22, 22, false));
    icon.addPixmap(KWindowSystem::icon(wid.toUInt(), 32, 32, false));
    icon.addPixmap(KWindowSystem::icon(wid.toUInt(), 48, 48, false));
    icon.addPixmap(KWindowSystem::icon(wid.toUInt(), 64, 64, false));
    icon.addPixmap(KWindowSystem::icon(wid.toUInt(), 128, 128, false));
    return icon;
}

QString XcbWindowManager::windowTitle(const QString &wid)
{
    const KWindowInfo winfo{wid.toUInt(), NET::WMName};
    return winfo.valid()? winfo.name() : QString{};
}

bool XcbWindowManager::skipTaskBar(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMState);
    return winfo.valid() && winfo.hasState(NET::SkipTaskbar);
}

QString XcbWindowManager::windowGroup(const QString &wid)
{
    KWindowInfo winfo(wid.toUInt(), 0, NET::WM2WindowClass);
    if(winfo.valid()) {
        return winfo.windowClassClass();
    }
    return QString();

}

bool XcbWindowManager::isMaximized(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMState);
    if(winfo.valid()) {
        return winfo.hasState(NET::MaxVert | NET::MaxHoriz);
    }
    return false;
}

void XcbWindowManager::maximizeWindow(const QString &wid)
{
    KWindowSystem::setState(wid.toUInt(), NET::Max);
}

bool XcbWindowManager::isMinimized(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMState);
    if(winfo.valid()) {
        return winfo.hasState(NET::Hidden);
    }
    return false;
}

void XcbWindowManager::minimizeWindow(const QString &wid)
{
    KWindowSystem::minimizeWindow(wid.toUInt());
}

bool XcbWindowManager::isKeepAbove(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMState);
    if(winfo.valid()) {
        return winfo.hasState(NET::KeepAbove);
    }
    return false;
}

void XcbWindowManager::keepAboveWindow(const QString &wid)
{
    if(isKeepAbove(wid)) {
        KWindowSystem::setState(wid.toUInt(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(wid.toUInt(), NET::KeepAbove);
    }
}

bool XcbWindowManager::isOnAllDesktops(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMDesktop);
    if(winfo.valid()) {
        return winfo.onAllDesktops();
    }
    return false;
}

bool XcbWindowManager::isOnCurrentDesktop(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMDesktop);
    if(winfo.valid()) {
        return winfo.isOnCurrentDesktop();
    }
    return false;
}

void XcbWindowManager::activateWindow(const QString &wid)
{
    KWindowSystem::activateWindow(wid.toUInt());
}

QString XcbWindowManager::currentActiveWindow()
{
    return QString::number(KWindowSystem::activeWindow());
}

void XcbWindowManager::closeWindow(const QString& wid)
{
    NETRootInfo ri(QX11Info::connection(), NET::CloseWindow);
    ri.closeWindowRequest(wid.toUInt());
}

void XcbWindowManager::restoreWindow(const QString &wid)
{
    KWindowSystem::clearState(wid.toUInt(), NET::Max);
}

void XcbWindowManager::windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    if (prop1 == 0 && !(prop2 & (NET::WM2Activities | NET::WM2TransientFor))) {
        return;
    }
    const KWindowInfo info(wid,prop1,prop2);
    if(prop1.testFlag(NET::WMState) && info.valid()) {
        if(info.hasState(NET::State::SkipTaskbar) != m_windowProperties.value(wid).value(SkipTaskbar).toBool()) {
            Q_EMIT skipTaskbarChanged(QString::number(wid));
        }
        if(info.hasState(NET::State::DemandsAttention) != m_windowProperties.value(wid).value(DemandsAttention).toBool()) {
            Q_EMIT demandsAttentionChanged(QString::number(wid));
        }
        if((info.hasState(NET::State::MaxHoriz) && info.hasState(NET::State::MaxVert)) != m_windowProperties.value(wid).value(Maximized).toBool()) {
            Q_EMIT maximizedChanged(QString::number(wid));
        }
        Q_EMIT windowStateChanged(QString::number(wid));
    }

    if(prop1.testFlag(NET::WMDesktop)) {
        if(info.isOnCurrentDesktop() != m_windowProperties.value(wid).value(OnCurrentDesktop).toBool()) {
            Q_EMIT windowDesktopChanged(QString::number(wid));
        }
        if(info.onAllDesktops() != m_windowProperties.value(wid).value(OnAllDesktops).toBool()) {
            Q_EMIT onAllDesktopsChanged(QString::number(wid));
        }
    }
    updateWindowProperties(wid);
    if(prop1.testFlag(NET::WMIcon)) {
        Q_EMIT iconChanged(QString::number(wid));
    }

    if(prop1.testFlag(NET::WMName) || prop1.testFlag(NET::WMVisibleName)) {
        Q_EMIT titleChanged(QString::number(wid));
    }

    if(prop1.testFlag(NET::WMGeometry)) {
        Q_EMIT geometryChanged(QString::number(wid));
    }
}

void XcbWindowManager::updateWindowProperties(WId wid)
{
    QHash<WindowProperty, QVariant> properties;
    const KWindowInfo winfo(wid, NET::WMState | NET::WMDesktop);
    properties.insert(WindowProperty::SkipTaskbar, winfo.valid() && winfo.hasState(NET::SkipTaskbar));
    properties.insert(WindowProperty::DemandsAttention, winfo.valid() && winfo.hasState(NET::DemandsAttention));
    properties.insert(WindowProperty::OnAllDesktops, winfo.valid() && winfo.onAllDesktops());
    properties.insert(WindowProperty::OnCurrentDesktop, winfo.valid() && winfo.isOnCurrentDesktop());
    properties.insert(WindowProperty::Maximized, winfo.valid() && winfo.hasState(NET::MaxHoriz) && winfo.hasState(NET::MaxVert));
    m_windowProperties.insert(wid, properties);
}

bool XcbWindowManager::isDemandsAttention(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMState);
    if(winfo.valid()) {
        return winfo.hasState(NET::DemandsAttention);
    }
    return false;
}

quint32 XcbWindowManager::pid(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMPid);
    if(winfo.valid()) {
        return winfo.pid();
    }
    return 0;
}

QString XcbWindowManager::appId(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), {}, NET::WM2DesktopFileName);
    if(winfo.valid()) {
        return winfo.desktopFileName();
    }
    return {};
}

QRect XcbWindowManager::geometry(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), NET::WMGeometry);
    if(winfo.valid()) {
        return winfo.geometry();
    }
    return {};
}

void XcbWindowManager::windowAddedProxy(WId wid)
{
    updateWindowProperties(wid);
    Q_EMIT windowAdded(QString::number(wid));
}

void XcbWindowManager::setStartupGeometry(const QString &wid, QQuickItem *item)
{
    if(!item || !item->window()) {
        return;
    }
    NETWinInfo info(QX11Info::connection(), wid.toInt(), item->window()->winId(), NET::WMIconGeometry, {}, NET::Client);
    info.setIconGeometry(QRect(item->mapToGlobal({0, 0}).toPoint(), item->size().toSize()));
}

void XcbWindowManager::setMinimizedGeometry(const QString &wid, QQuickItem *item)
{
    setStartupGeometry(wid, item);
}

void XcbWindowManager::unsetMinimizedGeometry(const QString &wid, QQuickItem *item)
{
}

bool XcbWindowManager::isMinimizable(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(),nullptr, NET::WM2AllowedActions);
    if(winfo.valid()) {
        return winfo.actionSupported(NET::ActionMax);
    }
    return false;
}

bool XcbWindowManager::isMaximizable(const QString &wid)
{
    const KWindowInfo winfo(wid.toUInt(), nullptr, NET::WM2AllowedActions);
    if(winfo.valid()) {
        return winfo.actionSupported(NET::ActionMinimize);
    }
    return false;
}
}