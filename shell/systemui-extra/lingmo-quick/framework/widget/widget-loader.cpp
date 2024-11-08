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

#include "widget-loader.h"
#include "widget.h"
#include "widget-container.h"

#include <QDir>
#include <QHash>

using namespace LingmoUIQuick;

namespace LingmoUIQuick {

class WidgetLoaderPrivate
{
public:
    explicit WidgetLoaderPrivate() {
        widgetSearchPath << ":/lingmo/widgets" << "/usr/share/lingmo/widgets" << (QDir::homePath() + "/.local/share/lingmo/widgets");
    }

    WidgetMetadata findMetadata(const QString &id) const {
        return m_metadataCache.value(id);
    }

    void cacheMetadata(const WidgetMetadata& metaData) {
        m_metadataCache.insert(metaData.id(), metaData);
//        if (!m_metadataCache.contains(metaData.id())) {
//            m_metadataCache.insert(metaData.id(), metaData);
//        }
    }

public:
    WidgetMetadata loadMetaData(const QString &id);
    QStringList widgetSearchPath;
    WidgetMetadata::Hosts hosts = WidgetMetadata::All;

private:
    QHash<QString, WidgetMetadata> m_metadataCache;
};
}

WidgetMetadata WidgetLoaderPrivate::loadMetaData(const QString &id)
{
    WidgetMetadata metaData = findMetadata(id);
    if (metaData.isValid()) {
        return metaData;
    }

    for (const auto &path : widgetSearchPath) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        QStringList subDirList = dir.entryList(QDir::Dirs);
        if (subDirList.contains(id)) {
            qDebug() << "loadMetaData for:" << id;
            metaData = WidgetMetadata(dir.absoluteFilePath(id));
            if (metaData.isValid()) {
                cacheMetadata(metaData);
                return metaData;
            }
            break;
        }
    }

    qWarning() << "can not find widget:" << id;
    return {};
}

WidgetLoader::WidgetLoader(QObject *parent) : QObject(parent), d(new WidgetLoaderPrivate())
{
}

Widget *WidgetLoader::loadWidget(const QString &id)
{
    WidgetMetadata metaData = loadMetadata(id);
    if (!metaData.isValid()) {
        return nullptr;
    }
    if (!(d->hosts & metaData.showIn())) {
        return nullptr;
    }

    Widget *widget = nullptr;
    if (metaData.widgetType() == Types::Container) {
        widget = new WidgetContainer(metaData);
    } else {
        widget = new Widget(metaData);
    }

    return widget;
}

WidgetMetadata WidgetLoader::loadMetadata(const QString &id)
{
    return d->loadMetaData(id);
}

WidgetLoader::~WidgetLoader()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void WidgetLoader::addWidgetSearchPath(const QString &path)
{
    if(!d->widgetSearchPath.contains(path) && !path.isEmpty()) {
        // 添加的自定义搜索路径优先级最高
        d->widgetSearchPath.prepend(path);
    }
}

WidgetLoader &WidgetLoader::globalLoader()
{
    static WidgetLoader loader;
    return loader;
}

void WidgetLoader::setShowInFilter(WidgetMetadata::Hosts hosts)
{
    d->hosts = hosts;
}
