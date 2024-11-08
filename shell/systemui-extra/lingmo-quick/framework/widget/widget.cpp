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

#include "widget.h"

#include <QPluginLoader>

#include "widget-container.h"
#include "config-loader.h"
#include "config-property-map.h"
#include "widget-interface.h"

namespace LingmoUIQuick {

class WidgetPrivate
{
public:
    explicit WidgetPrivate(const WidgetMetadata& m);

    int instanceId {-1};

    QString icon;
    QString name;
    QString tooltip;
    QString version;

    QString uiError;

    WidgetContent content;
    WidgetMetadata metaData;

    // actions, 用于显示当前Widget的某些操作
    QList<QAction *> actions;
    //附加action，用于Widget给container的action list或者container给widget的action list附加action
    //例如，widget可以通过widgetInterface插件形式给container的action列表中附加action，container给他的某些字widget附加一个和
    //container相关的action
    QList<QAction *> appendActions;
    bool active {false};
    bool visible {true};

    ConfigPropertyMap *qmlLocalConfig {nullptr};
    ConfigPropertyMap *qmlGlobalConfig {nullptr};

    Config *config {nullptr};
    WidgetContainer *container {nullptr};
    Types::Orientation orientation {Types::Horizontal};

    QScopedPointer<WidgetInterface> plugin;
};

WidgetPrivate::WidgetPrivate(const WidgetMetadata &m)
    : metaData(m), content(m),
      icon(m.icon()),
      name(m.name()),
      tooltip(m.tooltip()),
      version(m.version())
{

}

/**
 * 1.从目录加载完整的metadata信息。
 * 2.实例化widget,初始化组件信息。
 * 3.初始化数据组件，图形组件
 * 4.
 * @param metaData
 * @param parent
 */
Widget::Widget(const WidgetMetadata& metaData, QObject *parent) : QObject(parent), d(new WidgetPrivate(metaData))
{
    qRegisterMetaType<LingmoUIQuick::Widget *>();
    qRegisterMetaType<LingmoUIQuick::Config *>();
    qRegisterMetaType<LingmoUIQuick::ConfigPropertyMap *>();
    if(d->content.pluginPreload()) {
        loadPlugin();
    }
}

Widget::~Widget()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

Types::WidgetType Widget::type() const
{
    return LingmoUIQuick::Types::Widget;
}

QString Widget::id() const
{
    return d->metaData.id();
}

QString Widget::icon() const
{
    return d->icon;
}

void Widget::setIcon(const QString &icon)
{
    d->icon = icon;
    Q_EMIT iconChanged();
}

QString Widget::name() const
{
    return d->name;
}

void Widget::setName(const QString &name)
{
    d->name = name;
    Q_EMIT nameChanged();
}

QString Widget::tooltip() const
{
    return d->tooltip;
}

void Widget::setTooltip(const QString &tooltip)
{
    d->tooltip = tooltip;
    Q_EMIT tooltipChanged();
}

QString Widget::version() const
{
    return d->version;
}

void Widget::setVersion(const QString &version)
{
    d->version = version;
    Q_EMIT versionChanged();
}

QString Widget::website() const
{
    return d->metaData.website();
}

QString Widget::bugReport() const
{
    return d->metaData.bugReport();
}

QString Widget::description() const
{
    return d->metaData.description();
}

QVariantList Widget::authors() const
{
    return d->metaData.authors();;
}

const WidgetMetadata &Widget::metadata() const
{
    return d->metaData;
}

const WidgetContent &Widget::content() const
{
    return d->content;
}

void Widget::setUiError(const QString &error)
{
    d->uiError = error;
}

bool Widget::hasUiError() const
{
    return !d->uiError.isEmpty();
}

QString Widget::uiError() const
{
    return d->uiError;
}

QList<QAction *> Widget::actions() const
{
    return d->actions + d->appendActions;
}

void Widget::setActions(const QList<QAction *>& actions)
{
    d->actions = actions;
}

void Widget::setAppendActions(QList<QAction*> actions)
{
    d->appendActions = std::move(actions);
}

QQmlListProperty<QAction> Widget::qmlActions()
{
    return {this, &d->actions};
}

int Widget::instanceId() const
{
    return d->instanceId;
}

void Widget::setInstanceId(const int instanceId)
{
    if (instanceId == d->instanceId) {
        return;
    }

    d->instanceId = instanceId;
}

bool Widget::active() const
{
    return d->active;
}

void Widget::setActive(bool active)
{
    if (d->active == active) {
        return;
    }

    d->active = active;
    Q_EMIT activeChanged();
}

Types::Orientation Widget::orientation() const
{
    return d->orientation;
}

void Widget::setOrientation(Types::Orientation o)
{
    if (d->orientation == o) {
        return;
    }

    d->orientation = o;
    Q_EMIT orientationChanged();
}

ConfigPropertyMap *Widget::qmlLocalConfig() const
{
    if (!d->qmlLocalConfig) {
        Config *config = Widget::config();
        if (config) {
            d->qmlLocalConfig = new ConfigPropertyMap(config, (QObject *) this);
        }
    }

    return d->qmlLocalConfig;
}

ConfigPropertyMap *Widget::qmlGlobalConfig() const
{
    if (content().configPolicy() == WidgetContent::LocalOnly) {
        return qmlLocalConfig();
    }

    if (!d->qmlGlobalConfig) {
        Config *config = globalConfig();
        if (config) {
            d->qmlGlobalConfig = new ConfigPropertyMap(config, (QObject *) this);
        }
    }

    return d->qmlGlobalConfig;
}

Config *Widget::config() const
{
    if (!d->config) {
        if (d->container) {
            d->config = d->container->config()->child(QStringLiteral("widgets"), d->instanceId);
        }
    }

    return d->config;
}

void Widget::setConfig(Config *config)
{
    if (d->config == config) {
        return;
    }

    d->config = config;
}

Config *Widget::globalConfig() const
{
    WidgetContent::ConfigPolicy policy = content().configPolicy();
    if (policy == WidgetContent::LocalOnly) {
        return config();
    }

    return ConfigLoader::getConfig(d->metaData.id(), ConfigLoader::Global);
}

WidgetContainer *Widget::container() const
{
    return d->container;
}

WidgetMetadata::Hosts Widget::showIn() const
{
    return d->metaData.showIn();
}

void Widget::setContainer(WidgetContainer *container)
{
    WidgetContainer *old = d->container;
    if (old) {
        old->disconnect(this, nullptr);
    }

    d->container = container;
    Q_EMIT containerChanged(old ? old->id() : "", d->container ? d->container->id() : "");

    if (d->container) {
        setOrientation(d->container->orientation());
        connect(d->container, &WidgetContainer::orientationChanged, this, [this] {
            setOrientation(d->container->orientation());
        });
    }
}

void Widget::loadPlugin() const
{
    if(d->plugin.isNull() && !d->content.plugin().isNull()) {
        QPluginLoader pluginLoader(d->content.plugin());
        if(d->content.pluginVersion() == WIDGET_INTERFACE_VERSION) {
            d->plugin.reset(dynamic_cast<WidgetInterface*>(pluginLoader.instance()));
            if(d->plugin.isNull()) {
                qWarning() << "Fail to load widget plugin:" << id() << d->content.plugin();
            }
        } else {
            qWarning() << "Plugin version unmached:" << id() << d->content.plugin();
        }
    }
}

bool Widget::visible() const
{
    return false;
}

void Widget::setVisible(bool visible)
{
    if (d->visible == visible) {
        return;
    }

    d->visible = visible;
    Q_EMIT visibleChanged();
}
WidgetInterface* Widget::plugin() const
{
    loadPlugin();
    return d->plugin.data();
}
} // LingmoUIQuick
