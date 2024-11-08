/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "lingmo-window.h"

#include <QMoveEvent>
#include <QResizeEvent>
#include <QGuiApplication>

namespace LingmoUIQuick {

class LINGMOWindow::Private
{
public:
    bool m_needSetupWhenExposed {true};
    bool m_removeTitleBar {false};
    bool m_skipTaskBar {false};
    bool m_skipSwitcher {false};

    WindowType::Type m_type {WindowType::Normal};

    WMInterface *wm {nullptr};
};

LINGMOWindow::LINGMOWindow(QWindow *parent) : QQuickWindow(parent), d(new Private)
{
    qRegisterMetaType<LingmoUIQuick::WindowType::Type>();
    d->wm = WManager::getWM(this);
}

LINGMOWindow::~LINGMOWindow()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void LINGMOWindow::setupProperty()
{
    d->wm->setWindowType(d->m_type);
    d->wm->setRemoveTitleBar(d->m_removeTitleBar);
    d->wm->setSkipTaskBar(d->m_skipTaskBar);
    d->wm->setSkipSwitcher(d->m_skipSwitcher);
}

QPoint LINGMOWindow::windowPosition() const
{
    return d->wm->position();
}

void LINGMOWindow::setWindowPosition(const QPoint &pos)
{
    d->wm->setPosition(pos);
}

WindowType::Type LINGMOWindow::windowType() const
{
    return d->m_type;
}

void LINGMOWindow::setWindowType(WindowType::Type type)
{
    if (type == d->m_type) {
        return;
    }

    d->m_type = type;
    d->wm->setWindowType(d->m_type);
}

bool LINGMOWindow::skipTaskBar() const
{
    return d->m_skipTaskBar;
}

bool LINGMOWindow::skipSwitcher() const
{
    return d->m_skipSwitcher;
}

bool LINGMOWindow::removeTitleBar() const
{
    return d->m_removeTitleBar;
}

void LINGMOWindow::setRemoveTitleBar(bool remove)
{
    if (d->m_removeTitleBar == remove) {
        return;
    }

    d->m_removeTitleBar = remove;
    d->wm->setRemoveTitleBar(remove);

    emit removeTitleBarChanged();
}

void LINGMOWindow::setSkipTaskBar(bool skip)
{
    if (d->m_skipTaskBar == skip) {
        return;
    }

    d->m_skipTaskBar = skip;
    d->wm->setSkipTaskBar(d->m_skipTaskBar);

    emit skipTaskBarChanged();
}

void LINGMOWindow::setSkipSwitcher(bool skip)
{
    if (d->m_skipSwitcher == skip) {
        return;
    }

    d->m_skipSwitcher = skip;
    d->wm->setSkipSwitcher(d->m_skipSwitcher);

    emit skipSwitcherChanged();
}

bool LINGMOWindow::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Expose:
            if (!d->m_needSetupWhenExposed || !isVisible()) {
                break;
            }
            d->m_needSetupWhenExposed = false;
        case QEvent::Show:
            setupProperty();
            break;
        case QEvent::Hide:
            d->m_needSetupWhenExposed = true;
            break;
        default:
            break;
    }

    return QQuickWindow::event(event);
}

void LINGMOWindow::moveEvent(QMoveEvent *event)
{
    if (WMInterface::isWayland()) {
        // relocation
        d->wm->setPosition(d->wm->position());
    }

    QWindow::moveEvent(event);
}

void LINGMOWindow::resizeEvent(QResizeEvent *event)
{
    QQuickWindow::resizeEvent(event);
}

// ====== LINGMOWindowExtension ====== //
LINGMOWindowExtension::LINGMOWindowExtension(QObject *parent) : QObject(parent), m_window(qobject_cast<LINGMOWindow*>(parent))
{

}

int LINGMOWindowExtension::x() const
{
    return m_window->windowPosition().x();
}

int LINGMOWindowExtension::y() const
{
    return m_window->windowPosition().y();
}

void LINGMOWindowExtension::setX(int x)
{
    QPoint wPos = m_window->windowPosition();
    if (x == wPos.x()) {
        return;
    }

    wPos.setX(x);
    m_window->setWindowPosition(wPos);

    emit xChanged(x);
}

void LINGMOWindowExtension::setY(int y)
{
    QPoint wPos = m_window->windowPosition();
    if (y == wPos.y()) {
        return;
    }

    wPos.setY(y);
    m_window->setWindowPosition(wPos);

    emit yChanged(y);
}

} // LingmoUIQuick
