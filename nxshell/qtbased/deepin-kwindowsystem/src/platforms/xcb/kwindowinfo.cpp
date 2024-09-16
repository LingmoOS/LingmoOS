/*
    This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 2007 Lubos Lunak (l.lunak@kde.org)
    Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kwindowinfo_p_x11.h"
#include "kwindowsystem.h"

#include <QDebug>
#include <netwm.h>
#include <kxerrorhandler_p.h>
#include <QX11Info>
#include <X11/Xatom.h>

// KWindowSystem::info() should be updated too if something has to be changed here
KWindowInfoPrivateX11::KWindowInfoPrivateX11(WId _win, NET::Properties properties, NET::Properties2 properties2)
    : KWindowInfoPrivate(_win, properties, properties2)
    , KWindowInfoPrivateDesktopFileNameExtension()
    , KWindowInfoPrivatePidExtension()
{
    installDesktopFileNameExtension(this);
    installPidExtension(this);

    KXErrorHandler handler;
    if (properties & NET::WMVisibleIconName) {
        properties |= NET::WMIconName | NET::WMVisibleName;    // force, in case it will be used as a fallback
    }
    if (properties & NET::WMVisibleName) {
        properties |= NET::WMName;    // force, in case it will be used as a fallback
    }
    if (properties2 & NET::WM2ExtendedStrut) {
        properties |= NET::WMStrut;    // will be used as fallback
    }
    if (properties & NET::WMWindowType) {
        properties2 |= NET::WM2TransientFor;    // will be used when type is not set
    }
    if ((properties & NET::WMDesktop) && KWindowSystem::mapViewport()) {
        properties |= NET::WMGeometry;    // for viewports, the desktop (workspace) is determined from the geometry
    }
    properties |= NET::XAWMState; // force to get error detection for valid()
    m_info.reset(new NETWinInfo(QX11Info::connection(), _win, QX11Info::appRootWindow(), properties, properties2));
    if (properties & NET::WMName) {
        if (m_info->name() && m_info->name()[ 0 ] != '\0') {
            m_name = QString::fromUtf8(m_info->name());
        } else {
            m_name = KWindowSystem::readNameProperty(_win, XA_WM_NAME);
        }
    }
    if (properties & NET::WMIconName) {
        if (m_info->iconName() && m_info->iconName()[ 0 ] != '\0') {
            m_iconic_name = QString::fromUtf8(m_info->iconName());
        } else {
            m_iconic_name = KWindowSystem::readNameProperty(_win, XA_WM_ICON_NAME);
        }
    }
    if (properties & (NET::WMGeometry | NET::WMFrameExtents)) {
        NETRect frame, geom;
        m_info->kdeGeometry(frame, geom);
        m_geometry.setRect(geom.pos.x, geom.pos.y, geom.size.width, geom.size.height);
        m_frame_geometry.setRect(frame.pos.x, frame.pos.y, frame.size.width, frame.size.height);
    }
    m_valid = !handler.error(false);   // no sync - NETWinInfo did roundtrips
}

KWindowInfoPrivateX11::~KWindowInfoPrivateX11()
{
}

bool KWindowInfoPrivateX11::valid(bool withdrawn_is_valid) const
{
    if (!m_valid) {
        return false;
    }
    if (!withdrawn_is_valid && mappingState() == NET::Withdrawn) {
        return false;
    }
    return true;
}

NET::States KWindowInfoPrivateX11::state() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMState)) {
        qWarning() << "Pass NET::WMState to KWindowInfo";
    }
#endif
    return m_info->state();
}

NET::MappingState KWindowInfoPrivateX11::mappingState() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::XAWMState)) {
        qWarning() << "Pass NET::XAWMState to KWindowInfo";
    }
#endif
    return m_info->mappingState();
}

NETExtendedStrut KWindowInfoPrivateX11::extendedStrut() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2ExtendedStrut)) {
        qWarning() << "Pass NET::WM2ExtendedStrut to KWindowInfo";
    }
#endif
    NETExtendedStrut ext = m_info->extendedStrut();
    NETStrut str = m_info->strut();
    if (ext.left_width == 0 && ext.right_width == 0 && ext.top_width == 0 && ext.bottom_width == 0
            && (str.left != 0 || str.right != 0 || str.top != 0 || str.bottom != 0)) {
        // build extended from simple
        if (str.left != 0) {
            ext.left_width = str.left;
            ext.left_start = 0;
            ext.left_end = XDisplayHeight(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
        if (str.right != 0) {
            ext.right_width = str.right;
            ext.right_start = 0;
            ext.right_end = XDisplayHeight(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
        if (str.top != 0) {
            ext.top_width = str.top;
            ext.top_start = 0;
            ext.top_end = XDisplayWidth(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
        if (str.bottom != 0) {
            ext.bottom_width = str.bottom;
            ext.bottom_start = 0;
            ext.bottom_end = XDisplayWidth(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
    }
    return ext;
}

NET::WindowType KWindowInfoPrivateX11::windowType(NET::WindowTypes supported_types) const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMWindowType)) {
        qWarning() << "Pass NET::WMWindowType to KWindowInfo";
    }
#endif
    if (!m_info->hasWindowType()) { // fallback, per spec recommendation
        if (transientFor() != XCB_WINDOW_NONE) {  // dialog
            if (supported_types & NET::DialogMask) {
                return NET::Dialog;
            }
        } else {
            if (supported_types & NET::NormalMask) {
                return NET::Normal;
            }
        }
    }
    return m_info->windowType(supported_types);
}

QString KWindowInfoPrivateX11::visibleNameWithState() const
{
    QString s = visibleName();
    if (isMinimized()) {
        s.prepend(QLatin1Char('('));
        s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfoPrivateX11::visibleName() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMVisibleName)) {
        qWarning() << "Pass NET::WMVisibleName to KWindowInfo";
    }
#endif
    return m_info->visibleName() && m_info->visibleName()[ 0 ] != '\0'
           ? QString::fromUtf8(m_info->visibleName()) : name();
}

QString KWindowInfoPrivateX11::name() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMName)) {
        qWarning() << "Pass NET::WMName to KWindowInfo";
    }
#endif
    return m_name;
}

QString KWindowInfoPrivateX11::visibleIconNameWithState() const
{
    QString s = visibleIconName();
    if (isMinimized()) {
        s.prepend(QLatin1Char('('));
        s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfoPrivateX11::visibleIconName() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMVisibleIconName)) {
        qWarning() << "Pass NET::WMVisibleIconName to KWindowInfo";
    }
#endif
    if (m_info->visibleIconName() && m_info->visibleIconName()[ 0 ] != '\0') {
        return QString::fromUtf8(m_info->visibleIconName());
    }
    if (m_info->iconName() && m_info->iconName()[ 0 ] != '\0') {
        return QString::fromUtf8(m_info->iconName());
    }
    if (!m_iconic_name.isEmpty()) {
        return m_iconic_name;
    }
    return visibleName();
}

QString KWindowInfoPrivateX11::iconName() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMIconName)) {
        qWarning() << "Pass NET::WMIconName to KWindowInfo";
    }
#endif
    if (m_info->iconName() && m_info->iconName()[ 0 ] != '\0') {
        return QString::fromUtf8(m_info->iconName());
    }
    if (!m_iconic_name.isEmpty()) {
        return m_iconic_name;
    }
    return name();
}

bool KWindowInfoPrivateX11::isOnDesktop(int _desktop) const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMDesktop)) {
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
    }
#endif
    if (KWindowSystem::mapViewport()) {
        if (onAllDesktops()) {
            return true;
        }
        return KWindowSystem::viewportWindowToDesktop(m_geometry) == _desktop;
    }
    return m_info->desktop() == _desktop || m_info->desktop() == NET::OnAllDesktops;
}

bool KWindowInfoPrivateX11::onAllDesktops() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMDesktop)) {
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
    }
#endif
    if (KWindowSystem::mapViewport()) {
        if (m_info->passedProperties() & NET::WMState) {
            return m_info->state() & NET::Sticky;
        }
        NETWinInfo info(QX11Info::connection(), win(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());
        return info.state() & NET::Sticky;
    }
    return m_info->desktop() == NET::OnAllDesktops;
}

int KWindowInfoPrivateX11::desktop() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMDesktop)) {
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
    }
#endif
    if (KWindowSystem::mapViewport()) {
        if (onAllDesktops()) {
            return NET::OnAllDesktops;
        }
        return KWindowSystem::viewportWindowToDesktop(m_geometry);
    }
    return m_info->desktop();
}

QStringList KWindowInfoPrivateX11::activities() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2Activities)) {
        qWarning() << "Pass NET::WM2Activities to KWindowInfo";
    }
#endif

    const QStringList result = QString::fromLatin1(m_info->activities()).split(
        QLatin1Char(','), QString::SkipEmptyParts);

    return result.contains(QStringLiteral(KDE_ALL_ACTIVITIES_UUID)) ?
        QStringList() : result;
}

QRect KWindowInfoPrivateX11::geometry() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMGeometry)) {
        qWarning() << "Pass NET::WMGeometry to KWindowInfo";
    }
#endif
    return m_geometry;
}

QRect KWindowInfoPrivateX11::frameGeometry() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMFrameExtents)) {
        qWarning() << "Pass NET::WMFrameExtents to KWindowInfo";
    }
#endif
    return m_frame_geometry;
}

WId KWindowInfoPrivateX11::transientFor() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2TransientFor)) {
        qWarning() << "Pass NET::WM2TransientFor to KWindowInfo";
    }
#endif
    return m_info->transientFor();
}

WId KWindowInfoPrivateX11::groupLeader() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2GroupLeader)) {
        qWarning() << "Pass NET::WM2GroupLeader to KWindowInfo";
    }
#endif
    return m_info->groupLeader();
}

QByteArray KWindowInfoPrivateX11::windowClassClass() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2WindowClass)) {
        qWarning() << "Pass NET::WM2WindowClass to KWindowInfo";
    }
#endif
    return m_info->windowClassClass();
}

QByteArray KWindowInfoPrivateX11::windowClassName() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2WindowClass)) {
        qWarning() << "Pass NET::WM2WindowClass to KWindowInfo";
    }
#endif
    return m_info->windowClassName();
}

QByteArray KWindowInfoPrivateX11::windowRole() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2WindowRole)) {
        qWarning() << "Pass NET::WM2WindowRole to KWindowInfo";
    }
#endif
    return m_info->windowRole();
}

QByteArray KWindowInfoPrivateX11::clientMachine() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2ClientMachine)) {
        qWarning() << "Pass NET::WM2ClientMachine to KWindowInfo";
    }
#endif
    return m_info->clientMachine();
}

bool KWindowInfoPrivateX11::actionSupported(NET::Action action) const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2AllowedActions)) {
        qWarning() << "Pass NET::WM2AllowedActions to KWindowInfo";
    }
#endif
    if (KWindowSystem::allowedActionsSupported()) {
        return m_info->allowedActions() & action;
    } else {
        return true;    // no idea if it's supported or not -> pretend it is
    }
}

// see NETWM spec section 7.6
bool KWindowInfoPrivateX11::isMinimized() const
{
    if (mappingState() != NET::Iconic) {
        return false;
    }
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if ((state() & NET::Hidden) != 0
            && (state() & NET::Shaded) == 0) {  // shaded may have NET::Hidden too
        return true;
    }
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return KWindowSystem::icccmCompliantMappingState() ? false : true;
}

QByteArray KWindowInfoPrivateX11::desktopFileName() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties2() & NET::WM2DesktopFileName)) {
        qWarning() << "Pass NET::WM2DesktopFileName to KWindowInfo";
    }
#endif
    return QByteArray(m_info->desktopFileName());
}

int KWindowInfoPrivateX11::pid() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(m_info->passedProperties() & NET::WMPid)) {
        qWarning() << "Pass NET::WMPid to KWindowInfo";
    }
#endif
    return m_info->pid();
}
