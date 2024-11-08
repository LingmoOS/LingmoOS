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

#ifndef LINGMO_QUICK_WIDGET_ITEM_H
#define LINGMO_QUICK_WIDGET_ITEM_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>

namespace LingmoUIQuick {

class Widget;
class WidgetItemPrivate;
class WidgetQuickItemPrivate;

/**
 * 一个WidgetItem的基础组成定义
 * 包含重要的部分
 * 提供上下文属性widgetItem用于访问Widget实例
 */
class WidgetQuickItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickWindow *widgetWindow READ widgetWindow NOTIFY widgetWindowChanged)
    Q_PROPERTY(bool adjustingLayout READ adjustingLayout WRITE setAdjustingLayout NOTIFY adjustingLayoutChanged)

public:
    explicit WidgetQuickItem(QQuickItem *parent = nullptr);
    ~WidgetQuickItem() override;

    static void registerTypes();
    static WidgetQuickItem *loadWidgetItem(Widget *widget, QQmlContext *pc = nullptr);

    Widget *widget() const;
    QQmlContext *context() const;
    QQuickWindow *widgetWindow() const;

    bool adjustingLayout() const;
    void setAdjustingLayout(bool adjustingLayout);

Q_SIGNALS:
    void widgetWindowChanged();
    void adjustingLayoutChanged();

protected:
    void classBegin() override;

private:
    WidgetQuickItemPrivate *d {nullptr};
};

/**
 * Widget的图形组件，基于QQuickItem(QML)
 * 包含图形ui全部信息
 * TODO: 定义二级界面组件等
 */
class WidgetItem : public WidgetQuickItem
{
    Q_OBJECT
public:
    explicit WidgetItem(QQuickItem *parent = nullptr);
    ~WidgetItem() override;

private:
    WidgetItemPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_ITEM_H
