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

#ifndef LINGMO_QUICK_ISLAND_H
#define LINGMO_QUICK_ISLAND_H

#include <QObject>
#include <QString>
#include "config.h"
#include "widget-container.h"

namespace LingmoUIQuick {

class IslandPrivate;

/**
 * @class Island 岛，存放着一堆'view'.
 * 一般情况下，island表示一个应用程序,只为这个应用加载配置文件,并设置基本的组信息,不负责其他业务逻辑，
 * 配置文件默认分组为："_views"
 *
 * 一个island有多个'View'。
 * 但是只能有一个view处于实例化状态，也就是只能有一个顶级View.
 *
 */
class Island : public QObject
{
    Q_OBJECT
public:
    /**
     * 构造方法
     * @param id island的id,任意字符串
     * @param app 当前应用名称
     */
    explicit Island(const QString &id, const QString &app, QObject *parent = nullptr);
    Island() = delete;
    ~Island() override;

    QString id() const;
    QString app() const;
    Config *config() const;

    /**
     * 加载配置文件中的一个view
     * @param id view的ID
     * @param emitChangeSignal 加载成功后是否发送信号(自动加载ui)
     * @return 加载成功返回true,否则返回false
     */
    bool loadMainView(const QString &id, bool emitChangeSignal = true);

    /**
     * 主view
     * 使用前应该进行判断
     * @return
     */
    WidgetContainer *mainView() const;

    /**
     * 手动设置一个主view，不用loader加载
     * @param view
     */
    void setMainView(WidgetContainer *view, bool emitChangeSignal = true);

    /**
     * 内部view的id列表
     * @return 全部view的id
     */
    QStringList views() const;

    /**
     * view的数量
     * @return
     */
    int viewCount() const;

    /**
     * 默认分组的组名
     * @return
     */
    static QString viewGroupName();

    /**
     * 默认分组的键名
     * @return
     */
    static QString viewGroupKey();

Q_SIGNALS:
    void mainViewChanged();

private:
    IslandPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ISLAND_H
