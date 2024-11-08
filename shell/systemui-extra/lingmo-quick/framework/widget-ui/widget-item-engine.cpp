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

#include "widget-item-engine.h"
#include "widget-item-context.h"
#include "widget.h"
#include "shared-engine-component.h"

namespace LingmoUIQuick {

class WidgetItemEnginePrivate
{
public:
    explicit WidgetItemEnginePrivate(WidgetItemEngine *itemEngine);
    ~WidgetItemEnginePrivate() = default;

    WidgetItemEngine *q {nullptr};
    // 每一个widget都有一个自己的上下文
    WidgetItemContext *rootContext {nullptr};
    SharedEngineComponent *component {nullptr};
};

WidgetItemEnginePrivate::WidgetItemEnginePrivate(WidgetItemEngine *itemEngine) : q(itemEngine)
{

}

WidgetItemEngine::WidgetItemEngine(Widget *widget, QQmlContext *pc, QObject *parent)
    : QObject(parent),
      d(new WidgetItemEnginePrivate(this))
{
    if (pc && pc->engine() == SharedEngineComponent::sharedEngine()) {
        d->rootContext = new WidgetItemContext(pc, widget, this);
    } else {
        d->rootContext = new WidgetItemContext(SharedEngineComponent::sharedEngine(), widget, this);
    }

    d->component = new SharedEngineComponent(d->rootContext, this);
}

WidgetItemEngine::~WidgetItemEngine()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QObject *WidgetItemEngine::mainItem()
{
    return d->component->rootObject();
}

void WidgetItemEngine::loadMainItem()
{
    const auto widget = d->rootContext->widget();
    const auto mainUrl = widget->content().fileUrl(WidgetContent::Main);

    if (mainUrl.isEmpty()) {
        qWarning() << "WidgetItemEngine::loadMainItem: The main file was not found..";
        widget->setUiError(QStringLiteral("The main file was not found."));
        return;
    }

    d->component->setSource(mainUrl);
}

const QQmlComponent *WidgetItemEngine::component() const
{
    return d->component->component();
}

const Widget *WidgetItemEngine::widget() const
{
    return d->rootContext->widget();
}

const QQmlEngine *WidgetItemEngine::engine() const
{
    return SharedEngineComponent::sharedEngine();
}

WidgetItemContext *WidgetItemEngine::rootContext() const
{
    return d->rootContext;
}

void WidgetItemEngine::initContextProperty()
{
    if (d->rootContext) {
        d->rootContext->registerProperty();
//        d->rootContext->loadContextData();
    }
}

} // LingmoUIQuick
