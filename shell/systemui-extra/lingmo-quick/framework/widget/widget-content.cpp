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

#include "widget-content.h"
#include "widget-metadata.h"

#include <QVariant>
#include <QDebug>

using namespace LingmoUIQuick;

namespace LingmoUIQuick {

class WidgetContentPrivate
{
    friend class WidgetContent;
public:
    explicit WidgetContentPrivate(const WidgetMetadata &m) : contentsRoot(m.root()), contents(m.contents())
    {

    }

private:
    QDir contentsRoot;
    QVariantMap contents;
};

}


WidgetContent::WidgetContent(const WidgetMetadata &metaData) : d(new WidgetContentPrivate(metaData))
{

}

WidgetContent::~WidgetContent()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QString WidgetContent::main() const
{
    if (d->contents.contains(QStringLiteral("Main"))) {
        return d->contents.value("Main").toString();
    }

    return {};
}

QString WidgetContent::data() const
{
    if (d->contents.contains(QStringLiteral("Data"))) {
        return d->contents.value(QStringLiteral("Data")).toString();
    }

    return {};
}

QString WidgetContent::i18n() const
{
    QVariant i18n = d->contents.value(QStringLiteral("I18n"));
    if (i18n.isValid()) {
        if (i18n.userType() == QMetaType::QString) {
            return i18n.toString();
        }
    }

    return {};
}

QString WidgetContent::config() const
{
    QVariant cfg = d->contents.value(QStringLiteral("Config"));
    if (cfg.isValid()) {
        if (cfg.userType() == QMetaType::QString) {
            return cfg.toString();
        }
    }

    return {};
}

QString WidgetContent::plugin() const
{
    QVariant plugin = d->contents.value(QStringLiteral("Plugin"));
    if (plugin.isValid()) {
        if (plugin.userType() == QMetaType::QString) {
            return plugin.toString();
        }
    }

    return {};
}

QString WidgetContent::pluginVersion() const
{
    QVariant pluginVersion = d->contents.value(QStringLiteral("PluginVersion"));
    if (pluginVersion.isValid()) {
        if (pluginVersion.userType() == QMetaType::QString) {
            return pluginVersion.toString();
        }
    }

    return {};
}

bool WidgetContent::pluginPreload() const
{
    QVariant pluginPreload = d->contents.value(QStringLiteral("PluginPreload"));
    if (pluginPreload.isValid()) {
        if (pluginPreload.userType() == QMetaType::Bool) {
            return pluginPreload.toBool();
        }
    }
    return false;
}

WidgetContent::ConfigPolicy WidgetContent::configPolicy() const
{
    QString cfg = config();
    if (!cfg.isEmpty() && cfg == QStringLiteral("LocalOnly")) {
        return LocalOnly;
    }

    return Null;
}

QString WidgetContent::rootPath() const
{
    return d->contentsRoot.absolutePath();
}

QString WidgetContent::filePath(const WidgetContent::ContentKey &key) const
{
    QString file;
    switch (key) {
        case Main:
            file = main();
            break;
        case I18n: {
            file = i18n();
            break;
        }
        case Plugin: {
            file = plugin();
            break;
        }
        default:
            break;
    }

    if (file.isEmpty()) {
        return {};
    }

    return d->contentsRoot.absoluteFilePath(file);
}

QUrl WidgetContent::fileUrl(const WidgetContent::ContentKey &key) const
{
    if (key == Config) {
        return {};
    }

    QString path = filePath(key);
    if (path.isEmpty()) {
        return {};
    }

    // NOTE: 在此处特殊处理qrc文件路径，将qrc格式的path(如：':/'形式)输出为qrc:/格式的Url。
    // 目的是让QQmlComponent可以自动识别同目录的其他组件。
    if (path.startsWith(QStringLiteral(":/"))) {
        return QUrl(path.prepend(QStringLiteral("qrc")));
    }

    return QUrl::fromLocalFile(path);
}
