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

#ifndef LINGMO_MENU_WIDGET_EXTENSION_H
#define LINGMO_MENU_WIDGET_EXTENSION_H

#include <QObject>
#include <QVariant>

namespace LingmoUIMenu {

class WidgetMetadata
{
    Q_GADGET
public:
    enum Key {
        Id = 0,
        Icon,
        Name,
        Tooltip,
        Version,
        Description,
        Main,
        Type,
        Flag,
        Data
    };
    Q_ENUM(Key)

    enum TypeValue {
        Widget = 0x01, /**> 显示在插件区域 */
        Button = 0x02, /**> 显示在侧边栏 */
        AppList = 0x04 /**> 显示在应用列表，默认会显示在全屏界面 */
    };
    Q_ENUM(TypeValue)
    Q_DECLARE_FLAGS(Types, TypeValue)
    Q_FLAGS(Types)

    enum FlagValue {
        OnlySmallScreen = 0x01,
        OnlyFullScreen = 0x02,
        Normal = OnlySmallScreen | OnlyFullScreen
    };
    Q_ENUM(FlagValue)
    Q_DECLARE_FLAGS(Flags, FlagValue)
    Q_FLAGS(Flags)
};

typedef QMap<LingmoUIMenu::WidgetMetadata::Key, QVariant> MetadataMap;

class WidgetExtension : public QObject
{
    Q_OBJECT
public:
    explicit WidgetExtension(QObject *parent = nullptr);
    virtual int index() const;
    virtual MetadataMap metadata() const = 0;

    // 兼容老版本
    virtual QVariantMap data();
    virtual void receive(const QVariantMap &data);

Q_SIGNALS:
    void dataUpdated();
};

} // LingmoUIMenu

Q_DECLARE_METATYPE(LingmoUIMenu::MetadataMap)
Q_DECLARE_METATYPE(LingmoUIMenu::WidgetMetadata::TypeValue)
Q_DECLARE_METATYPE(LingmoUIMenu::WidgetMetadata::FlagValue)

#endif //LINGMO_MENU_WIDGET_EXTENSION_H
