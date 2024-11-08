/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LINGMO_MENU_CONTEXT_MENU_EXTENSION_H
#define LINGMO_MENU_CONTEXT_MENU_EXTENSION_H

#include <QObject>
#include <QMenu>
#include "data-entity.h"

namespace LingmoUIMenu {

class MenuInfo
{
    Q_GADGET
public:
    enum Location {
        AppList = 0, /**< 小屏幕的应用列表 */
        Extension, /**< 扩展页 */
        FolderPage, /**< 文件夹页面 */
        FullScreen, /**< 全屏应用列表 */
        Folder, /**< 应用组页面 */
    };
    Q_ENUM(Location)
};

/**
 * @class ContextMenuExtension
 *
 * 开始菜单应用列表的上下文菜单扩展插件
 */
class ContextMenuExtension
{
public:
    virtual ~ContextMenuExtension() = default;
    /**
     * 控制菜单项显示在哪个位置
     * 对于第三方项目们应该选择-1,或者大于1000的值
     * @return -1：表示随机放在最后
     */
    virtual int index() const;

    /**
     * 根据data生成action,或者子菜单
     *
     * @param data app信息
     * @param parent action最终显示的QMenu
     * @param location 请求菜单的位置
     * @param locationId 位置的描述信息，可选的值有：all,category,letterSort和favorite等插件的id
     * @return
     */
    virtual QList<QAction*> actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location, const QString &locationId) = 0;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_CONTEXT_MENU_EXTENSION_H
