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

#ifndef LINGMO_QUICK_WIDGET_ITEM_ATTACHED_H
#define LINGMO_QUICK_WIDGET_ITEM_ATTACHED_H

#include <QObject>
#include <qqml.h>

namespace LingmoUIQuick {

class Widget;

class WidgetItemAttached : public QObject
{
    Q_OBJECT
//    QML_ATTACHED(WidgetItemAttached)
public:
    static Widget *qmlAttachedProperties(QObject *object);

    explicit WidgetItemAttached(QObject *parent=nullptr);
};

} // LingmoUIQuick

// 添加附加属性
QML_DECLARE_TYPEINFO(LingmoUIQuick::WidgetItemAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif //LINGMO_QUICK_WIDGET_ITEM_ATTACHED_H
