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

#ifndef LINGMO_QUICK_WIDGET_ITEM_ENGINE_H
#define LINGMO_QUICK_WIDGET_ITEM_ENGINE_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlComponent>

namespace LingmoUIQuick {

class Widget;
class WidgetItemContext;
class WidgetItemEnginePrivate;

/**
 * @class WidgetItemEngine
 *
 * WidgetItemEngine 是用于为一个Widget创建Item的工具。
 * 内部为每一个Item创建了一个上下文，并挂载了一些常见数据。
 *
 * 调用loadRootObject方法自动解析main.qml加载根对象。
 *
 */
class WidgetItemEngine : public QObject
{
    Q_OBJECT
    friend class WidgetItemEnginePrivate;
public:
    explicit WidgetItemEngine(Widget *widget, QQmlContext *pc = nullptr, QObject *parent = nullptr);
    ~WidgetItemEngine() override;

    // 所属的源Widget
    const Widget *widget() const;
    // 当前运行在的engine
    const QQmlEngine *engine() const;
    // item的上下文
    WidgetItemContext *rootContext() const;

    // 根节点对象
    QObject *mainItem();
    // 自动解析当前Widget的Metadata并通过Component加载主入口(main.qml)文件。
    void loadMainItem();

    // 根节点的Component，可用于clone生成一个object
    const QQmlComponent *component() const;

    // 将Widget中定义的上下文数据挂载到上下文
    void initContextProperty();

private:
    WidgetItemEnginePrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_ITEM_ENGINE_H
