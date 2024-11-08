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

#include "widget-container.h"

#include <QJsonArray>

#include "widget.h"
#include "island.h"

namespace LingmoUIQuick {

class WidgetContainerPrivate
{
public:
    WidgetContainerPrivate(WidgetContainer *q) : q(q){};
    bool containActiveWidgets();
    void setDisableInstancesId(const QList<int>& instanceIds, bool removeDumplicated = true);
    void loadConfig(Config* config);
    void removeDisableInstances();

    // prop
    int radius {0};
    QRect geometry;
    QScreen *screen {nullptr};
    Margin *margin {nullptr};
    Margin *padding {nullptr};
    Types::Pos position {Types::NoPosition};

    // widgets
    QVector<Widget *> widgets;
    Island *island {nullptr};
    WidgetMetadata::Host host = WidgetMetadata::All;
    bool configLoaded = false;
    QList<int> disableInstances;
    WidgetContainer *q = nullptr;
};

bool WidgetContainerPrivate::containActiveWidgets()
{
    return std::any_of(widgets.constBegin(), widgets.constEnd(), [] (Widget *widget) {
        return widget->active();
    });
}

void WidgetContainerPrivate::setDisableInstancesId(const QList<int>& instanceIds, bool removeDumplicated)
{
    //去重
    if(removeDumplicated) {
        QHash<int, bool> uniqueElements;
        foreach(int element, instanceIds)
        {
            uniqueElements[element] = true;
        }
        disableInstances.clear();
        disableInstances.reserve(uniqueElements.size());
        for (auto it = uniqueElements.begin(); it != uniqueElements.end(); ++it)
        {
            disableInstances.append(it.key());
        }
    } else {
        disableInstances = instanceIds;
    }

    QJsonArray jsonArray;
    for (int value : disableInstances) {
        jsonArray.append(value);
    }
    q->config()->setValue(QStringLiteral("disabledInstances"), jsonArray);
}

void WidgetContainerPrivate::loadConfig(Config* config)
{
    if (config && !configLoaded) {
        auto list = config->children(QStringLiteral("widgets"));
        auto array = config->getValue(QStringLiteral("disabledInstances")).toJsonArray();
        disableInstances.clear();
        for(auto value : array) {
            disableInstances.append(value.toInt());
        }
        for (const auto widgetConfig: list) {
            if(disableInstances.contains(widgetConfig->getValue(QStringLiteral("instanceId")).toInt())) {
                continue;
            }
            q->addWidget(widgetConfig->getValue(QStringLiteral("id")).toString(),
                      widgetConfig->getValue(QStringLiteral("instanceId")).toInt());
        }
        configLoaded = true;
    }
}

void WidgetContainerPrivate::removeDisableInstances()
{
    for(Widget *w : widgets) {
        if(disableInstances.contains(w->instanceId())) {
            q->removeWidget(w);
        }
    }
}

WidgetContainer::WidgetContainer(QObject *parent)
    : WidgetContainer({}, parent) {}

WidgetContainer::WidgetContainer(const WidgetMetadata &metaData, QObject *parent)
    : Widget(metaData, parent), d(new WidgetContainerPrivate(this))
{
    d->margin = new Margin(this);
    d->padding = new Margin(this);

    qRegisterMetaType<LingmoUIQuick::WidgetContainer *>();
}

WidgetContainer::~WidgetContainer()
{
    for(auto w : d->widgets) {
        removeWidget(w);
    }
    if(d) {
        delete d;
        d = nullptr;
    }
}

Types::WidgetType WidgetContainer::type() const
{
    return Types::Container;
}

QString WidgetContainer::appid() const
{
    if (d->island) {
        return d->island->app();
    }

    return {};
}

void WidgetContainer::addWidget(Widget *widget)
{
    if (!widget || d->widgets.contains(widget) || d->disableInstances.contains(widget->instanceId())) {
        return;
    }

    // 删除重复instanceId的widget
    auto it = std::find_if(d->widgets.constBegin(), d->widgets.constEnd(), [widget] (const Widget *w) {
        return w->instanceId() == widget->instanceId();
    });

    if (it != d->widgets.constEnd()) {
        removeWidget(*it);
    }
    connect(widget, &Widget::activeChanged, this, &Widget::activeChanged);
    d->widgets.append(widget);
    if (config() && !config()->child(QStringLiteral("widgets"), widget->instanceId())) {
        QVariantMap wData;
        wData.insert(QStringLiteral("id"), widget->id());
        wData.insert(QStringLiteral("instanceId"), widget->instanceId());
        config()->addChild(QStringLiteral("widgets"), wData);
    }

    widget->setContainer(this);
    Q_EMIT widgetAdded(widget);
}

void WidgetContainer::addWidget(const QString &id, const int instanceId)
{
    if (instanceId < 0 || d->disableInstances.contains(instanceId)) {
        return;
    }

    Widget *widget = WidgetContainer::widgetLoader().loadWidget(id);
    if (widget) {
        widget->setInstanceId(instanceId);
        addWidget(widget);
    }
}

WidgetLoader &WidgetContainer::widgetLoader()
{
    return WidgetLoader::globalLoader();
}

void WidgetContainer::removeWidget(QString id)
{
    for(Widget *w : d->widgets) {
        if(w->id() == id) {
            removeWidget(d->widgets.indexOf(w));
        }
    }
}

void WidgetContainer::removeWidget(Widget *widget)
{
    if (!widget) {
        return;
    }

    removeWidget(d->widgets.indexOf(widget));
}

void WidgetContainer::removeWidget(int index)
{
    if (index < 0 || index >= d->widgets.size()) {
        return;
    }

    Widget *widget = d->widgets.takeAt(index);
    // 通知ContainerItem卸载WidgetItem,Widget即将被删除
    Q_EMIT widgetRemoved(widget);

    // 发送Widget删除信号，这个操作会从全局的缓存中移除widget对应的WidgetItem并delete掉它
    Q_EMIT widget->aboutToDeleted();
    widget->disconnect(this);
    widget->deleteLater();
}

QRect WidgetContainer::geometry() const
{
    return d->geometry;
}

void WidgetContainer::setGeometry(QRect geometry)
{
    if (d->geometry == geometry) {
        return;
    }

    d->geometry = geometry;
    Q_EMIT geometryChanged();
}

Margin* WidgetContainer::margin() const
{
    return d->margin;
}

Margin *WidgetContainer::padding() const
{
    return d->padding;
}

QScreen *WidgetContainer::screen() const
{
    return d->screen;
}

void WidgetContainer::setScreen(QScreen *screen)
{
    if(d->screen == screen) {
        return;
    }
    d->screen = screen;
    Q_EMIT screenChanged();
}

int WidgetContainer::radius() const
{
    return d->radius;
}

void WidgetContainer::setRadius(int radius)
{
    if (d->radius == radius) {
        return;
    }

    d->radius = radius;
    Q_EMIT radiusChanged();
}

Types::Pos WidgetContainer::position() const
{
    return d->position;
}

void WidgetContainer::setPosition(Types::Pos position)
{
    if (d->position == position) {
        return;
    }

    d->position = position;
    Q_EMIT positionChanged();
}

QVector<Widget *> WidgetContainer::widgets() const
{
    return d->widgets;
}

bool WidgetContainer::active() const
{
    if(d->containActiveWidgets()) {
        return true;
    }
    return Widget::active();
}

WidgetMetadata::Host WidgetContainer::host() const
{
    return d->host;
}

void WidgetContainer::setHost(WidgetMetadata::Host host)
{
    if(d->host == host) {
        return;
    }
    d->host = host;
    widgetLoader().setShowInFilter(d->host);
    Q_EMIT hostChanged();
}

const Island *WidgetContainer::island() const
{
    return d->island;
}

void WidgetContainer::setIsland(Island *island)
{
    if (d->island == island) {
        return;
    }

    d->island = island;
    auto config = island->config()->child(LingmoUIQuick::Island::viewGroupName(), instanceId());
    if (config) {
        setConfig(config);
        d->loadConfig(this->config());
    }
}

void WidgetContainer::setContainer(WidgetContainer* container)
{
    Widget::setContainer(container);
    setScreen(container->screen());
    connect(container, &LingmoUIQuick::WidgetContainer::screenChanged, this, [&, container](){
        setScreen(container->screen());
    });

    setPosition(container->position());
    connect(container, &LingmoUIQuick::WidgetContainer::positionChanged, this, [&, container](){
        setPosition(container->position());
    });

    setConfig(container->config()->child(QStringLiteral("widgets"), instanceId()));
    setInstanceId(config()->getValue(QStringLiteral("instanceId")).toInt());

    d->loadConfig(config());
}

void WidgetContainer::setConfig(Config* config)
{
    Widget::setConfig(config);
    if(config) {
        config->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
    }
}

void WidgetContainer::setDisableInstances(const QList<int>& instanceIds)
{
    d->setDisableInstancesId(instanceIds);
    d->removeDisableInstances();
}

void WidgetContainer::addDisableInstance(int id)
{
    if(d->disableInstances.contains(id)) {
        return;
    }
    d->setDisableInstancesId(d->disableInstances << id, false);
    d->removeDisableInstances();
}

void WidgetContainer::removeDisableInstance(int id)
{
    if(!d->disableInstances.contains(id)) {
        return;
    }
    d->disableInstances.removeAll(id);
    d->setDisableInstancesId(d->disableInstances, false);
}

QList<int> WidgetContainer::disableInstances()
{
    return d->disableInstances;
}
} // LingmoUIQuick
