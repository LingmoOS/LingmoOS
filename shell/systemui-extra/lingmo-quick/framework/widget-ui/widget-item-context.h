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

#ifndef LINGMO_QUICK_WIDGET_ITEM_CONTEXT_H
#define LINGMO_QUICK_WIDGET_ITEM_CONTEXT_H

#include <QQmlContext>

namespace LingmoUIQuick {

class Widget;
class WidgetItemEngine;

/**
 * @class WidgetItemContext
 *
 * 每一个WidgetItem的上下文
 * 可以为Item挂载自定义的数据
 *
 */
class WidgetItemContext : public QQmlContext
{
    Q_OBJECT
    friend class WidgetItemEngine;
public:
    explicit WidgetItemContext(QQmlEngine *engine, Widget *widget, QObject *objParent = nullptr);
    explicit WidgetItemContext(QQmlContext *parent, Widget *widget, QObject *objParent = nullptr);

    Widget *widget() const;

private:
    void init();
    void registerProperty();
    void loadContextData();

private:
    Widget *m_widget {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_ITEM_CONTEXT_H
