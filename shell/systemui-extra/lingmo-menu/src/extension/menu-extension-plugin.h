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

#ifndef LINGMO_MENU_MENU_EXTENSION_PLUGIN_H
#define LINGMO_MENU_MENU_EXTENSION_PLUGIN_H

#define LINGMO_MENU_EXTENSION_I_FACE_TYPE     "LINGMO_MENU_EXTENSION"
#define LINGMO_MENU_EXTENSION_I_FACE_IID      "org.lingmo.menu.extension"
#define LINGMO_MENU_EXTENSION_I_FACE_VERSION  "1.0.2"

#include <QtPlugin>

namespace LingmoUIMenu {

class WidgetExtension;
class ContextMenuExtension;

class Q_DECL_EXPORT MenuExtensionPlugin : public QObject
{
    Q_OBJECT
public:
    explicit MenuExtensionPlugin(QObject *parent = nullptr);
    ~MenuExtensionPlugin() override;

    /**
     * 插件的唯一id，会被用于区分插件
     * @return 唯一id
     */
    virtual QString id() = 0;

    /**
     * 创建一个Widget扩展
     * @return 返回nullptr代表不生产此插件
     */
    virtual WidgetExtension *createWidgetExtension() = 0;

    /**
     * 创建上下文菜单扩展
     * @return 返回nullptr代表不生产此插件
     */
    virtual ContextMenuExtension *createContextMenuExtension() = 0;
};

} // LingmoUIMenu

Q_DECLARE_INTERFACE(LingmoUIMenu::MenuExtensionPlugin, LINGMO_MENU_EXTENSION_I_FACE_IID)

#endif //LINGMO_MENU_MENU_EXTENSION_PLUGIN_H
