/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#include <QWindow>
#include <QQuickWindow>
#include "menu.h"

namespace LingmoUIQuick {

Menu::Menu(QObject *prent) : QObject(prent)
{
    m_menu = new QMenu(nullptr);
    connect(m_menu, &QMenu::aboutToHide, this, [=]() {
        Q_EMIT visibleChanged();
    });
    connect(m_menu, &QMenu::aboutToShow, this, [=]() {
        Q_EMIT visibleChanged();
    });
}

Menu::~Menu()
{
    delete m_menu;
}

QQmlListProperty<MenuItem> Menu::content()
{
    return QQmlListProperty<MenuItem>(this, &m_items);
}

void Menu::open(int x, int y)
{
    m_menu->clear();
    for (MenuItem *item : m_items) {
        m_menu->addAction(item->action());
    }

    if(m_transientParent) {
        m_menu->winId();
        if(m_menu->windowHandle()) {
            m_menu->windowHandle()->setTransientParent(m_transientParent->window());
        }
    }
    if (x < 0 || y < 0) {
        m_menu->popup(QCursor::pos());
    } else {
        m_menu->popup(QPoint(x, y));
    }
}

void Menu::close()
{
    m_menu->hide();
}

bool Menu::visible()
{
    return m_menu->isVisible();
}

void Menu::setTransientParent(QQuickItem *item)
{
    if(m_transientParent == item) {
        return;
    }
    m_transientParent = item;
    Q_EMIT transientParentChanged();
}
}
