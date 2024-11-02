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

#ifndef LINGMO_QUICK_ISLAND_VIEW_H
#define LINGMO_QUICK_ISLAND_VIEW_H

#include <QQuickWindow>
#include <QQmlContext>
#include <QUrl>

#include "widget-container-item.h"
#include "island.h"

namespace LingmoUIQuick {

class IslandViewPrivate;

/**
 * 1.用于加载ContainerItem的窗口
 *
 * 封装加载逻辑，简化加载流程
 */
class IslandView : public QQuickWindow
{
    Q_OBJECT
public:
    IslandView() = delete;
    explicit IslandView(const QString &id, const QString &app);
    ~IslandView() override;

    /**
     * 在搜索路径中自动寻找该插件并加载，现已实现
     */
    bool loadMainView(const QString &id);

    /**
     * 在搜索路径中自动寻找该插件并加载,不加载UI,配合bool loadMainViewItem()使用
     */
    bool loadMainViewWithoutItem(const QString &id);

    /**
     * 加载当前mainview的ui
     */
    bool loadMainViewItem();

    /**
     * 设置一个Container作为根View，一般用作异常情况调用
     * @param view
     */
    void setMainView(WidgetContainer *view);
    WidgetContainer *mainView() const;

    /**
     * 已废弃接口，使用 @loadMainView(const QString &id); 自动加载。
     * @note: 若需要分离Container与island层，可以从此处加载特定的island代码，并将Container嵌入到该对象。
     *
     * 加载Container的ui
     * 不同的Container往往UI也不相同，需要按情况编写并加载
     * @param source ui文件的地址
     */
    [[deprecated]] void setSource(const QUrl &source);

    /**
     * mainView UI组件的上下文
     * 目前的情况是，islandView拥有一个上下文，用于开发时向ui提供一些数据
     * Container也有一个上下文，他们两个之间的数据并不互通，为了能让这两个上下文互通，现在islandView的上下文会作为Container的父级存在。
     */
    QQmlContext *rootContext() const;
    WidgetContainerItem *rootItem() const;

    Config *config() const;
    Island *island() const;

private:
    IslandViewPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ISLAND_VIEW_H
