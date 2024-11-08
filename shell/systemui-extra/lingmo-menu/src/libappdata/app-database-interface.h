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

#ifndef LINGMO_MENU_APP_DATABASE_INTERFACE_H
#define LINGMO_MENU_APP_DATABASE_INTERFACE_H

#include <QObject>
#include <QPair>
#include "data-entity.h"

namespace LingmoUIMenu {

typedef QVector<DataEntity> DataEntityVector;

/**
 * 封装与应用数据库的链接
 */
class AppDatabaseWorkerPrivate;

class AppDatabaseInterface : public QObject
{
    Q_OBJECT
public:
    explicit AppDatabaseInterface(QObject *parent = nullptr);

    /**
     * 从数据库获取全部应用
     * @return
     */
    DataEntityVector apps() const;
    /**
     * 从数据库获取单个应用
     * @param appid 应用id
     * @param app 获取应用
     */
    bool getApp(const QString &appid, DataEntity &app) const;

    /**
     * 置顶应用
     * @param appid 应用id
     * @param index 置顶后的位置，小于或等于0将会取消置顶
     */
    void fixAppToTop(const QString &appid, int index) const;

    /**
     * 收藏应用
     * @param appid 应用id
     * @param index 收藏后的位置，小于或等于0将会取消收藏
     */
    void fixAppToFavorite(const QString &appid, int index) const;

    /**
     * 设置应用是否已经被启动过
     * @param state 状态值，true已被启动，false未被启动
     */
    void updateApLaunchedState(const QString &appid, bool state = true) const;

Q_SIGNALS:
    /**
     * 新增应用信号
     * @param apps 新增加的应用
     */
    void appAdded(const LingmoUIMenu::DataEntityVector &apps);

    /**
     * 更新的应用及其属性列表
     * @param updates
     */
    void appUpdated(const QVector<QPair<LingmoUIMenu::DataEntity, QVector<int> > > &updates);

    /**
     * 应用被删除的信号
     * @param apps 返回被删除应用的id列表
     */
    void appDeleted(const QStringList &apps);

    /**
     * 应用数据库打开失败信号
     */
    void appDatabaseOpenFailed();

private:
    AppDatabaseWorkerPrivate *d {nullptr};
};

} // LingmoUIMenu

Q_DECLARE_METATYPE(LingmoUIMenu::DataEntityVector)

#endif //LINGMO_MENU_APP_DATABASE_INTERFACE_H
