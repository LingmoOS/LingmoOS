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

#include "island-view.h"

#include "shared-engine-component.h"
#include "island.h"

namespace LingmoUIQuick {

class IslandViewPrivate
{
public:
    explicit IslandViewPrivate(IslandView *q);

    Island *island {nullptr};
    QQmlContext *rootContext {nullptr};
    WidgetContainerItem *containerItem {nullptr};
    IslandView *q = nullptr;
    void onContentItemSizeChanged();
    bool onMainViewChanged();
};

IslandViewPrivate::IslandViewPrivate(IslandView* q) : q(q)
{
}
/**
 * 主视图的尺寸始终与窗口尺寸保持一致
 */
void IslandViewPrivate::onContentItemSizeChanged()
{
    if (containerItem) {
        containerItem->setSize(q->contentItem()->size());
    }
}

bool IslandViewPrivate::onMainViewChanged()
{
    auto cont = island->mainView();
    if (!cont) {
        return false;
    }

    auto contItem = qobject_cast<WidgetContainerItem *>(WidgetQuickItem::loadWidgetItem(cont, rootContext));
    if (!contItem) {
        qWarning() << "The main item must be WidgetContainerItem.";
        return false;
    }

    containerItem = contItem;
    containerItem->setParentItem(q->contentItem());
    onContentItemSizeChanged();
    return true;
}

IslandView::IslandView(const QString &id, const QString &app) : QQuickWindow(), d(new IslandViewPrivate(this))
{
    // TODO: 信号监听
    d->island = new Island(id, app, this);
    d->rootContext = new QQmlContext(SharedEngineComponent::sharedEngine(), this);
    d->rootContext->setContextProperty("island", this);

    // 默认将mainView的大小调整为窗口大小
    QObject::connect(contentItem(), &QQuickItem::widthChanged, this, [this]() {
        d->onContentItemSizeChanged();
    });
    QObject::connect(contentItem(), &QQuickItem::heightChanged, this, [this]() {
        d->onContentItemSizeChanged();
    });

    // 当island指定的主View切换时，同步加载对应的主qml文件
    connect(d->island, &Island::mainViewChanged, this, [this] {
        d->onMainViewChanged();
    });
}

IslandView::~IslandView()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void IslandView::setSource(const QUrl &source)
{
    Q_UNUSED(source)
}

QQmlContext *IslandView::rootContext() const
{
    return d->rootContext;
}

WidgetContainerItem *IslandView::rootItem() const
{
    return d->containerItem;
}

bool IslandView::loadMainView(const QString &id)
{
    return d->island->loadMainView(id);
}

bool IslandView::loadMainViewWithoutItem(const QString &id)
{
    return d->island->loadMainView(id, false);
}

bool IslandView::loadMainViewItem()
{
    return d->onMainViewChanged();
}

void IslandView::setMainView(WidgetContainer *container)
{
    d->island->setMainView(container);
}

WidgetContainer *IslandView::mainView() const
{
    return d->island->mainView();
}

Config *IslandView::config() const
{
    return d->island->config();
}

Island *IslandView::island() const
{
    return d->island;
}
} // LingmoUIQuick
