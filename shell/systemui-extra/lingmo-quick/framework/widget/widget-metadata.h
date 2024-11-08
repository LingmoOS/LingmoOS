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

#ifndef LINGMO_QUICK_WIDGET_METADATA_H
#define LINGMO_QUICK_WIDGET_METADATA_H

#include <QDir>
#include <QString>
#include <QVariant>
#include <QJsonObject>

#include "types.h"

namespace LingmoUIQuick {
class WidgetMetadata
{
public:
    enum MetadataKey
    {
        Authors,
        Id,
        Icon,
        Name,
        Description,
        Version,
        Website,
        BugReport,
        Contents,
        ShowIn
    };

    enum Host
    {
        Undefined = 0x00000000,
        /**
         *@brief 用于panel类型container，如lingmo-panel
         */
        Panel = 0x00000001,
        /**
         *@brief 用于sidebar类型container,如lingmo-sidebar
         */
        SideBar = 0x00000002,
        /**
         *@brief 用于desktop类型container
         */
        Desktop = 0x00000004,
        /**
         *@brief 可以加载到taskManager的图标上
         */
        TaskManager = 0x00000008,
        /**
         *@brief 用于所有类型container
         */
        All = Panel | SideBar | Desktop | TaskManager
    };

    Q_DECLARE_FLAGS(Hosts, Host);

    WidgetMetadata() = default;
    explicit WidgetMetadata(const QString &root);

public:
    bool isValid() const;
    const QDir &root() const;

    QString id() const;
    QString icon() const;
    QString name() const;
    QString tooltip() const;
    QString version() const;
    QString website() const;
    QString bugReport() const;
    QString description() const;
    QVariantList authors() const;
    QVariantMap contents() const;
    Hosts showIn() const;
    Types::WidgetType widgetType() const;

private:
    void init();
    static QString localeKey(const QString &key);

private:
    bool m_isValid {false};
    QString m_id;

    QDir m_root;
    QJsonObject m_object;
};

} // LingmoUIQuick
Q_DECLARE_OPERATORS_FOR_FLAGS(LingmoUIQuick::WidgetMetadata::Hosts)
#endif //LINGMO_QUICK_WIDGET_METADATA_H
