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

#ifndef LINGMO_QUICK_WIDGET_CONTENT_H
#define LINGMO_QUICK_WIDGET_CONTENT_H

#include <QUrl>
#include <QString>

namespace LingmoUIQuick {

class WidgetMetadata;
class WidgetContentPrivate;

/**
 * widget包内的文件映射
 */
class WidgetContent
{
public:
    enum ContentKey {
        Main,
        Config,
        I18n,
        Plugin,
        PluginVersion,
        PluginPreload
    };

    enum ConfigPolicy {
        Null,
        LocalOnly
    };

    WidgetContent() = delete;
    explicit WidgetContent(const WidgetMetadata &metaData);
    virtual ~WidgetContent();
    QString main() const;
    QString data() const;
    QString i18n() const;
    QString config() const;
    QString plugin() const;
    QString pluginVersion() const;
    bool pluginPreload() const;
    ConfigPolicy configPolicy() const;

    // Widget的根目录
    QString rootPath() const;

    // 获取文件的绝对路径
    QString filePath(const ContentKey &key) const;

    // 获取文件文件的Url
    QUrl fileUrl(const ContentKey &key) const;

private:
    WidgetContentPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_CONTENT_H
