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

#include "island.h"
#include "config-loader.h"

#include <QDebug>
#include <utility>

namespace LingmoUIQuick {

class IslandPrivate
{
public:
    explicit IslandPrivate(QString id, QString app);

    // 当前Island的Id,可以是任意字符串
    QString islandId;

    // island所在的app名称, 一般是进程名称, 如: lingmo-panel, lingmo-menu等.
    QString appId;

    // 当前Island的主配置文件
    Config *config {nullptr};

    // island当前加载的顶级View, 有且只有一个顶级View
    WidgetContainer *view {nullptr};
};

IslandPrivate::IslandPrivate(QString id, QString app) : islandId(std::move(id)), appId(std::move(app))
{
}


Island::Island(const QString &id, const QString &app, QObject *parent)
    : QObject(parent), d(new IslandPrivate(id, app))
{
    // _root
    auto config = ConfigLoader::getConfig(id, ConfigLoader::Local, app);
    if (!config) {
        config = new Config(QVariantMap(), this);
        qWarning() << "Island: can not load config for:" << id << app << ", use default.";
    }

    // 初始配置分组信息
    config->addGroupInfo(Island::viewGroupName(), Island::viewGroupKey());
    d->config = config;
}

Island::~Island()
{
    if (d->view) {
        Q_EMIT d->view->aboutToDeleted();
        d->view->deleteLater();
        d->view = nullptr;
    }
    if (d) {
        delete d;
        d = nullptr;
    }
}

Config *Island::config() const
{
    return d->config;
}

QString Island::id() const
{
    return d->islandId;
}

QString Island::app() const
{
    return d->appId;
}

WidgetContainer *Island::mainView() const
{
    return d->view;
}

bool Island::loadMainView(const QString &id, bool emitChangeSignal)
{
    auto list = d->config->children(viewGroupName());
    Config *config = nullptr;
    for(auto c : list) {
        if(c->getValue(QStringLiteral("id")) == id) {
            config = c;
        }
    }

    if (!config) {
        qWarning() << "Config for:" << id << "not found, genarating default one";
        QVariantMap data;
        data.insert(QStringLiteral("id"), id);
        int instanceId = d->config->children(LingmoUIQuick::Island::viewGroupName()).count();
        data.insert(QStringLiteral("instanceId"), instanceId);
        data.insert(QStringLiteral("widgets"), QVariantList());
        d->config->addChild(viewGroupName(), data);
        config = d->config->child(viewGroupName(), instanceId);
    }

    // TODO: 从一个'widget包'进行加载
    auto widget = WidgetContainer::widgetLoader().loadWidget(id);
    auto container = qobject_cast<WidgetContainer *>(widget);
    if (!container) {
        qWarning() << "The main view is not 'Container'" << id;
        delete widget;
        return false;
    }

    container->setInstanceId(config->getValue(QStringLiteral("instanceId")).toInt());
    container->setConfig(config);
    setMainView(container, emitChangeSignal);

    return true;
}

void Island::setMainView(WidgetContainer *view, bool emitChangeSignal)
{
    if (view == d->view) {
        return;
    }

    if (d->view) {
        Q_EMIT d->view->aboutToDeleted();
        d->view->deleteLater();
        d->view = nullptr;
    }

    d->view = view;
    if (view) {
        view->setIsland(this);
    }

    if(emitChangeSignal) {
        Q_EMIT mainViewChanged();
    }
}

QString Island::viewGroupName()
{
    return QStringLiteral("_views");
}

QString Island::viewGroupKey()
{
    return QStringLiteral("instanceId");
}

QStringList Island::views() const
{
    QStringList list;
    for (const auto &item : d->config->children(Island::viewGroupName())) {
        list << item->id().toString();
    }

    return list;
}

int Island::viewCount() const
{
    return d->config->numberOfChildren(Island::viewGroupName());
}

} // LingmoUIQuick
