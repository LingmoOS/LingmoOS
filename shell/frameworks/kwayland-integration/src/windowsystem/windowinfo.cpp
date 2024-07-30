/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich (ettrich@kde.org)
    SPDX-FileCopyrightText: 2007 Lubos Lunak (l.lunak@kde.org)
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "windowinfo.h"

#include "kwindowsystem.h"
#include "waylandintegration.h"

#include <config-kwindowsystem.h>

#include <QGuiApplication>
#include <QRect>

#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

WindowInfo::WindowInfo(WId window, NET::Properties properties, NET::Properties2 properties2)
    : KWindowInfoPrivate(window, properties, properties2)
    , m_valid(false)
    , m_properties(properties)
    , m_properties2(properties2)
    , m_surface(KWayland::Client::Surface::fromQtWinId(window))
    , m_plasmaShellSurface(KWayland::Client::PlasmaShellSurface::get(m_surface))
{
    m_valid = m_surface != nullptr && m_surface->isValid();
}

WindowInfo::~WindowInfo()
{
}

bool WindowInfo::valid(bool withdrawn_is_valid) const
{
    Q_UNUSED(withdrawn_is_valid)
    return m_valid;
}

NET::States WindowInfo::state() const
{
    return {};
}

bool WindowInfo::isMinimized() const
{
    return false;
}

NET::MappingState WindowInfo::mappingState() const
{
    return NET::Visible;
}

NETExtendedStrut WindowInfo::extendedStrut() const
{
    return NETExtendedStrut();
}

NET::WindowType WindowInfo::windowType(NET::WindowTypes supported_types) const
{
    if (!m_plasmaShellSurface || !m_plasmaShellSurface->isValid()) {
        return NET::Unknown;
    }

    if (m_properties & NET::WMWindowType) {
        switch (m_plasmaShellSurface->role()) {
        case KWayland::Client::PlasmaShellSurface::Role::Normal:
            if (supported_types & NET::NormalMask) {
                return NET::Normal;
            }
            break;
        case KWayland::Client::PlasmaShellSurface::Role::Desktop:
            if (supported_types & NET::DesktopMask) {
                return NET::Desktop;
            }
            break;
        case KWayland::Client::PlasmaShellSurface::Role::Panel:
            if (supported_types & NET::DockMask) {
                return NET::Dock;
            }
            break;
        case KWayland::Client::PlasmaShellSurface::Role::OnScreenDisplay:
            if (supported_types & NET::OnScreenDisplayMask) {
                return NET::OnScreenDisplay;
            }
            break;
        case KWayland::Client::PlasmaShellSurface::Role::Notification:
            if (supported_types & NET::NotificationMask) {
                return NET::Notification;
            }
            break;
        case KWayland::Client::PlasmaShellSurface::Role::ToolTip:
            if (supported_types & NET::TooltipMask) {
                return NET::Tooltip;
            }
            break;
        case KWayland::Client::PlasmaShellSurface::Role::CriticalNotification:
            if (supported_types & NET::CriticalNotificationMask) {
                return NET::CriticalNotification;
            }
            break;
        default:
            break;
        }
    }

    return NET::Unknown;
}

QString WindowInfo::visibleName() const
{
    return QString();
}

QString WindowInfo::visibleNameWithState() const
{
    return QString();
}

QString WindowInfo::name() const
{
    return QString();
}

QString WindowInfo::visibleIconName() const
{
    return QString();
}

QString WindowInfo::visibleIconNameWithState() const
{
    return QString();
}

QString WindowInfo::iconName() const
{
    return QString();
}

bool WindowInfo::onAllDesktops() const
{
    return false;
}

bool WindowInfo::isOnDesktop(int desktop) const
{
    Q_UNUSED(desktop)
    return false;
}

int WindowInfo::desktop() const
{
    return 0;
}

QStringList WindowInfo::activities() const
{
    return QStringList();
}

QRect WindowInfo::geometry() const
{
    return QRect();
}

QRect WindowInfo::frameGeometry() const
{
    return QRect();
}

WId WindowInfo::transientFor() const
{
    return 0;
}

WId WindowInfo::groupLeader() const
{
    return 0;
}

QByteArray WindowInfo::windowClassClass() const
{
    return QByteArray();
}

QByteArray WindowInfo::windowClassName() const
{
    return QByteArray();
}

QByteArray WindowInfo::windowRole() const
{
    return QByteArray();
}

QByteArray WindowInfo::clientMachine() const
{
    return QByteArray();
}

bool WindowInfo::actionSupported(NET::Action action) const
{
    Q_UNUSED(action)
    return false;
}
