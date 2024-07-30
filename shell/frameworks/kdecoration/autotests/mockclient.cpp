/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "mockclient.h"
#include <decoratedclient.h>

#include <QPalette>

MockClient::MockClient(KDecoration2::DecoratedClient *client, KDecoration2::Decoration *decoration)
    : QObject()
    , ApplicationMenuEnabledDecoratedClientPrivate(client, decoration)
{
}

Qt::Edges MockClient::adjacentScreenEdges() const
{
    return Qt::Edges();
}

QString MockClient::caption() const
{
    return QString();
}

WId MockClient::decorationId() const
{
    return 0;
}

int MockClient::height() const
{
    return m_height;
}

QIcon MockClient::icon() const
{
    return QIcon();
}

bool MockClient::isActive() const
{
    return false;
}

bool MockClient::isCloseable() const
{
    return m_closeable;
}

bool MockClient::isKeepAbove() const
{
    return m_keepAbove;
}

bool MockClient::isKeepBelow() const
{
    return m_keepBelow;
}

bool MockClient::isMaximizeable() const
{
    return m_maximizable;
}

bool MockClient::isMaximized() const
{
    return isMaximizedHorizontally() && isMaximizedVertically();
}

bool MockClient::isMaximizedHorizontally() const
{
    return m_maximizedHorizontally;
}

bool MockClient::isMaximizedVertically() const
{
    return m_maximizedVertically;
}

bool MockClient::isMinimizeable() const
{
    return m_minimizable;
}

bool MockClient::isModal() const
{
    return false;
}

bool MockClient::isMoveable() const
{
    return false;
}

bool MockClient::isOnAllDesktops() const
{
    return m_onAllDesktops;
}

bool MockClient::isResizeable() const
{
    return false;
}

bool MockClient::isShadeable() const
{
    return m_shadeable;
}

bool MockClient::isShaded() const
{
    return m_shaded;
}

QPalette MockClient::palette() const
{
    return QPalette();
}

bool MockClient::hasApplicationMenu() const
{
    return true;
}

bool MockClient::isApplicationMenuActive() const
{
    return false;
}

bool MockClient::providesContextHelp() const
{
    return m_contextHelp;
}

void MockClient::requestClose()
{
    Q_EMIT closeRequested();
}

void MockClient::requestContextHelp()
{
    Q_EMIT quickHelpRequested();
}

void MockClient::requestToggleMaximization(Qt::MouseButtons buttons)
{
    bool maximizedHorizontally = m_maximizedHorizontally;
    bool maximizedVertically = m_maximizedVertically;
    if (buttons.testFlag(Qt::LeftButton)) {
        maximizedHorizontally = !m_maximizedHorizontally;
        maximizedVertically = !m_maximizedVertically;
    }
    if (buttons.testFlag(Qt::MiddleButton)) {
        maximizedHorizontally = !m_maximizedHorizontally;
    }
    if (buttons.testFlag(Qt::RightButton)) {
        maximizedVertically = !m_maximizedVertically;
    }
    const bool wasMaximized = isMaximized();
    if (m_maximizedHorizontally != maximizedHorizontally) {
        m_maximizedHorizontally = maximizedHorizontally;
        Q_EMIT client()->maximizedHorizontallyChanged(m_maximizedHorizontally);
    }
    if (m_maximizedVertically != maximizedVertically) {
        m_maximizedVertically = maximizedVertically;
        Q_EMIT client()->maximizedVerticallyChanged(m_maximizedVertically);
    }
    if (wasMaximized != isMaximized()) {
        Q_EMIT client()->maximizedChanged(isMaximized());
    }
}

void MockClient::requestMinimize()
{
    Q_EMIT minimizeRequested();
}

void MockClient::requestShowWindowMenu(const QRect &rect)
{
    Q_EMIT menuRequested();
}

void MockClient::requestShowApplicationMenu(const QRect &rect, int actionId)
{
    Q_UNUSED(rect);
    Q_UNUSED(actionId);
    Q_EMIT applicationMenuRequested(); // FIXME TODO pass geometry
}

void MockClient::requestToggleKeepAbove()
{
    m_keepAbove = !m_keepAbove;
    Q_EMIT client()->keepAboveChanged(m_keepAbove);
}

void MockClient::requestToggleKeepBelow()
{
    m_keepBelow = !m_keepBelow;
    Q_EMIT client()->keepBelowChanged(m_keepBelow);
}

void MockClient::requestToggleOnAllDesktops()
{
    m_onAllDesktops = !m_onAllDesktops;
    Q_EMIT client()->onAllDesktopsChanged(m_onAllDesktops);
}

void MockClient::requestToggleShade()
{
    m_shaded = !m_shaded;
    Q_EMIT client()->shadedChanged(m_shaded);
}

void MockClient::requestShowToolTip(const QString &text)
{
    Q_UNUSED(text);
}

void MockClient::requestHideToolTip()
{
}

QSize MockClient::size() const
{
    return QSize(m_width, m_height);
}

int MockClient::width() const
{
    return m_width;
}

QString MockClient::windowClass() const
{
    return QString();
}

WId MockClient::windowId() const
{
    return 0;
}

void MockClient::setCloseable(bool set)
{
    m_closeable = set;
    Q_EMIT client()->closeableChanged(set);
}

void MockClient::setMinimizable(bool set)
{
    m_minimizable = set;
    Q_EMIT client()->minimizeableChanged(set);
}

void MockClient::setProvidesContextHelp(bool set)
{
    m_contextHelp = set;
    Q_EMIT client()->providesContextHelpChanged(set);
}

void MockClient::setShadeable(bool set)
{
    m_shadeable = set;
    Q_EMIT client()->shadeableChanged(set);
}

void MockClient::setMaximizable(bool set)
{
    m_maximizable = set;
    Q_EMIT client()->maximizeableChanged(set);
}

void MockClient::setWidth(int w)
{
    m_width = w;
    Q_EMIT client()->widthChanged(w);
}

void MockClient::setHeight(int h)
{
    m_height = h;
    Q_EMIT client()->heightChanged(h);
}

void MockClient::showApplicationMenu(int actionId)
{
    Q_UNUSED(actionId)
}

#include "moc_mockclient.cpp"
