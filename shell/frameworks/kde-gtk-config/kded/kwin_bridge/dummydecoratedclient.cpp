/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "dummydecoratedclient.h"

namespace KDecoration2
{
DummyDecoratedClient::DummyDecoratedClient(DecoratedClient *client, Decoration *decoration)
    : DecoratedClientPrivate(client, decoration)
    , m_pallete(QStringLiteral("kdeglobals"))
    , m_maximized()
    , m_active(true)
{
}

bool DummyDecoratedClient::isActive() const
{
    return m_active;
}

QString DummyDecoratedClient::caption() const
{
    return {};
}

bool DummyDecoratedClient::isOnAllDesktops() const
{
    return true;
}

bool DummyDecoratedClient::isShaded() const
{
    return true;
}

QIcon DummyDecoratedClient::icon() const
{
    return {};
}

bool DummyDecoratedClient::isMaximized() const
{
    return m_maximized;
}

bool DummyDecoratedClient::isMaximizedHorizontally() const
{
    return m_maximized;
}

bool DummyDecoratedClient::isMaximizedVertically() const
{
    return m_maximized;
}

bool DummyDecoratedClient::isKeepAbove() const
{
    return true;
}

bool DummyDecoratedClient::isKeepBelow() const
{
    return true;
}

bool DummyDecoratedClient::isCloseable() const
{
    return true;
}

bool DummyDecoratedClient::isMaximizeable() const
{
    return true;
}

bool DummyDecoratedClient::isMinimizeable() const
{
    return true;
}

bool DummyDecoratedClient::providesContextHelp() const
{
    return true;
}

bool DummyDecoratedClient::isModal() const
{
    return true;
}

bool DummyDecoratedClient::isShadeable() const
{
    return true;
}

bool DummyDecoratedClient::isMoveable() const
{
    return true;
}

bool DummyDecoratedClient::isResizeable() const
{
    return true;
}

WId DummyDecoratedClient::windowId() const
{
    return {};
}

WId DummyDecoratedClient::decorationId() const
{
    return {};
}

int DummyDecoratedClient::width() const
{
    return {};
}

int DummyDecoratedClient::height() const
{
    return {};
}

QSize DummyDecoratedClient::size() const
{
    return {};
}

QPalette DummyDecoratedClient::palette() const
{
    return m_pallete.palette();
}

QColor DummyDecoratedClient::color(ColorGroup group, ColorRole role) const
{
    return m_pallete.color(group, role);
}

Qt::Edges DummyDecoratedClient::adjacentScreenEdges() const
{
    return {};
}

QString DummyDecoratedClient::windowClass() const
{
    return {};
}

void DummyDecoratedClient::requestShowToolTip(const QString &text)
{
    Q_UNUSED(text)
}

void DummyDecoratedClient::requestHideToolTip()
{
}

void DummyDecoratedClient::requestClose()
{
}

void DummyDecoratedClient::requestToggleMaximization(Qt::MouseButtons buttons)
{
    Q_UNUSED(buttons)
    m_maximized = !m_maximized;
}

void DummyDecoratedClient::requestMinimize()
{
}

void DummyDecoratedClient::requestContextHelp()
{
}

void DummyDecoratedClient::requestToggleOnAllDesktops()
{
}

void DummyDecoratedClient::requestToggleShade()
{
}

void DummyDecoratedClient::requestToggleKeepAbove()
{
}

void DummyDecoratedClient::requestToggleKeepBelow()
{
}

void DummyDecoratedClient::requestShowWindowMenu(const QRect &rect)
{
    Q_UNUSED(rect)
}

void DummyDecoratedClient::setMaximized(bool maximized)
{
    m_maximized = maximized;
}

void DummyDecoratedClient::setActive(bool active)
{
    m_active = active;
}

}

#include "moc_dummydecoratedclient.cpp"
