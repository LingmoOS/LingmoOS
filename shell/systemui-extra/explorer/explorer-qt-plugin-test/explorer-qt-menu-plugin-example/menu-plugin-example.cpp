/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "menu-plugin-example.h"

#include <QAction>
#include <QMenu>

#include <QDebug>

using namespace Peony;

MenuPluginExample::MenuPluginExample(QObject *parent) : QObject(parent)
{
}

QString MenuPluginExample::testPlugin()
{
    qDebug()<<"menu test plugin1";
    return QString("MenuPluginExample");
}

QList<QAction *> MenuPluginExample::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
    //return QList<QAction *>();
    Q_UNUSED(types);
    Q_UNUSED(uri);
    Q_UNUSED(selectionUris);
    QList<QAction *> actions;
    QAction *action = new QAction(QIcon::fromTheme("search"), tr("plugin-sub-menu test"));

    actions<<action;
    QMenu *menu = new QMenu(action->parentWidget());
    connect(action, &QAction::destroyed, [=]() {
        qDebug()<<"delete sub menu";
        menu->deleteLater();
    });
    menu->addAction("sub test1");
    menu->addSeparator();
    menu->addAction("sub test2");
    action->setMenu(menu);
    QAction *action2 = new QAction(QIcon::fromTheme("media-eject"), tr("plugin-action test"));
    connect(action2, &QAction::triggered, [=]() {
        qDebug()<<"action triggered";
    });
    actions<<action2;

    return actions;
}
