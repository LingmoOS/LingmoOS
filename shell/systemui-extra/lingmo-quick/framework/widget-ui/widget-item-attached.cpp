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

#include "widget-item-attached.h"
#include "widget.h"
#include "widget-item.h"
#include "widget-item-context.h"

#include <QQmlEngine>
#include <QQmlContext>

namespace LingmoUIQuick {

Widget *WidgetItemAttached::qmlAttachedProperties(QObject *object)
{
    auto widgetItem = qobject_cast<WidgetItem *>(object);
    if (widgetItem) {
        return widgetItem->widget();
    }

    QQmlContext *context = QQmlEngine::contextForObject(object);
    while (context) {
        context = context->parentContext();
        if (auto c = qobject_cast<WidgetItemContext *>(context)) {
            return c->widget();
        }
    }

    return nullptr;
}

WidgetItemAttached::WidgetItemAttached(QObject *parent) : QObject(parent)
{

}

} // LingmoUIQuick
