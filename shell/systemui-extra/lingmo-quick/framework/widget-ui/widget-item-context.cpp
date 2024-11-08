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

#include "widget-item-context.h"
#include "widget-item-engine.h"
#include "widget.h"

namespace LingmoUIQuick {

WidgetItemContext::WidgetItemContext(QQmlEngine *engine, Widget *widget, QObject *objParent)
  : QQmlContext(engine, objParent), m_widget(widget)
{
    init();
}

WidgetItemContext::WidgetItemContext(QQmlContext *parent, Widget *widget, QObject *objParent)
  : QQmlContext(parent, objParent), m_widget(widget)
{
    init();
}

void WidgetItemContext::init()
{
    // init context property
    setContextProperty(QStringLiteral("widget"), m_widget);
    if (m_widget->type() == Types::Container) {
        setContextProperty(QStringLiteral("container"), m_widget);
    }
}

void WidgetItemContext::loadContextData()
{
    // TODO: data接口
}

void WidgetItemContext::registerProperty()
{
    //
}

Widget *WidgetItemContext::widget() const
{
    return m_widget;
}

} // LingmoUIQuick
