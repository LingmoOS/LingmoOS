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

#ifndef LINGMO_QUICK_ITEMS_MENU_H
#define LINGMO_QUICK_ITEMS_MENU_H

#include "menu-item.h"
#include <QMenu>
#include <QObject>
#include <QQmlListProperty>
#include <QQuickItem>

namespace LingmoUIQuick {

/**
 * Example :
 * import org.lingmo.quick.items 1.0
 * MouseArea {
        Menu {
            id: menu
            transientParent: parent
            content: [
                MenuItem {
                    text: "Click!!!!"
                    onClicked: console.log("menu:Clicked!!")
                }
            ]
        }
        onClicked: {
            menu.open();
        }
  }
 */

class Menu : public QObject
{
    Q_OBJECT
    // 错误
    // Q_PROPERTY(QQmlListProperty<MenuItem> content READ content)
    // 正确
    Q_PROPERTY(QQmlListProperty<LingmoUIQuick::MenuItem> content READ content)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    Q_PROPERTY(QQuickItem* transientParent MEMBER m_transientParent WRITE setTransientParent NOTIFY transientParentChanged)

public:
    explicit Menu(QObject *prent = nullptr);
    ~Menu();
    QQmlListProperty<MenuItem> content();
    bool visible();

    Q_INVOKABLE void open(int x = -1, int y = -1);
    Q_INVOKABLE void close();
    void setTransientParent(QQuickItem* item);

private:
    QList<MenuItem *> m_items;
    QMenu *m_menu = nullptr;
    QQuickItem *m_transientParent = nullptr;

Q_SIGNALS:
    void visibleChanged();
    void transientParentChanged();
};

}

#endif //LINGMO_QUICK_ITEMS_MENU_H
