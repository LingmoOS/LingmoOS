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

#include "widget-item.h"
#include "widget-item-engine.h"
#include "widget-item-context.h"
#include "widget-item-attached.h"

#include "widget.h"
#include "widget-container.h"
#include "widget-container-item.h"

#include <QHash>
#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>
#include <qqml.h>

#include <mutex>

std::once_flag flag;

using namespace LingmoUIQuick;

namespace LingmoUIQuick {

class WidgetQuickItemPrivate
{
public:
    Widget *widget{nullptr};
    WidgetItemEngine *itemEngine{nullptr};
    bool adjustingLayout {false};

    static QHash<QString, QTranslator *> s_translators;
    static QHash<Widget *, WidgetQuickItem *> s_widgetItems;

    static void installTranslator(const Widget *widget);
    static void unInstallTranslator(const QString &widgetId);

    static WidgetQuickItem *toWidgetItem(Widget *widget, QObject *object);
    static WidgetQuickItem *toContainerItem(Widget *widget, QObject *object);
};

QHash<QString, QTranslator *> WidgetQuickItemPrivate::s_translators = QHash<QString, QTranslator *>();
QHash<Widget *, WidgetQuickItem *> WidgetQuickItemPrivate::s_widgetItems = QHash<Widget *, WidgetQuickItem *>();

// 加载翻译文件
void WidgetQuickItemPrivate::installTranslator(const Widget *widget)
{
    /**
     * TODO: 安装多个翻译文件
     * 将翻译文件安装到规定的路径下，通过唯一id找到翻译文件并加载，实现为某组件加载多个翻译文件
     */
    const QString wid = widget->id();
    if (!WidgetQuickItemPrivate::s_translators.contains(wid)) {
        // i18n可以是数组
        QString tsFile = widget->content().i18n();
        if (!tsFile.isEmpty()) {
            auto translator = new QTranslator(QCoreApplication::instance());
            bool loaded = translator->load(QLocale(), tsFile, QStringLiteral("_"), widget->content().rootPath());
            if (loaded) {
                QCoreApplication::installTranslator(translator);
                WidgetQuickItemPrivate::s_translators.insert(wid, translator);
                qDebug() << "Installing translator for:" << wid << QLocale();
            } else {
                qWarning() << "Widget:" << wid << ",Load translation file for" << QLocale::system().name() << "failed!";
                delete translator;
            }
        }
    }
}

void WidgetQuickItemPrivate::unInstallTranslator(const QString &widgetId)
{
    // 卸载翻译
    auto it = std::find_if(WidgetQuickItemPrivate::s_widgetItems.constBegin(),
                           WidgetQuickItemPrivate::s_widgetItems.constEnd(), [&widgetId](const WidgetQuickItem *wi) {
        return wi->widget()->id() == widgetId;
    });

    if (it == WidgetQuickItemPrivate::s_widgetItems.constEnd()) {
        auto translator = WidgetQuickItemPrivate::s_translators.take(widgetId);
        if (translator) {
            QCoreApplication::removeTranslator(translator);
            delete translator;
        }
    }
}

WidgetQuickItem *WidgetQuickItemPrivate::toWidgetItem(Widget *widget, QObject *object)
{
    auto widgetItem = qobject_cast<WidgetItem *>(object);
    if (!widgetItem) {
        // 转换错误，qml的根项不是WidgetItem.
        qWarning() << "Error: The root item must be WidgetItem.";
        widget->setUiError("Error: The root item must be WidgetItem.");
        return nullptr;
    }

    widgetItem->setProperty("widget", QVariant::fromValue(widget));
    widget->setProperty("widgetItem", QVariant::fromValue(widgetItem));

    return widgetItem;
}

WidgetQuickItem *WidgetQuickItemPrivate::toContainerItem(Widget *widget, QObject *object)
{
    auto containerItem = qobject_cast<WidgetContainerItem *>(object);
    if (!containerItem) {
        qWarning() << "Error: The root item must be WidgetContainerItem.";
        widget->setUiError("Error: The root item must be WidgetContainerItem.");
        return nullptr;
    }

    containerItem->setProperty("container", QVariant::fromValue(widget));
    widget->setProperty("containerItem", QVariant::fromValue(containerItem));

    return containerItem;
}

// ====== WidgetQuickItem ====== //
WidgetQuickItem::WidgetQuickItem(QQuickItem *parent) : QQuickItem(parent), d(new WidgetQuickItemPrivate)
{
    connect(this, &QQuickItem::windowChanged, this, &WidgetQuickItem::widgetWindowChanged);
}

WidgetQuickItem::~WidgetQuickItem()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void WidgetQuickItem::registerTypes()
{
    // 注册widget和container
    const char *uri = "org.lingmo.quick.widgets";
    const char *containerUri = "org.lingmo.quick.container";

    qmlRegisterType<WidgetItem>(uri, 1, 0, "WidgetItem");
    qmlRegisterUncreatableType<WidgetItemAttached>(uri, 1, 0, "Widget", "For widgetItem attached prop.");

    qmlRegisterType<WidgetContainerItem>(containerUri, 1, 0, "WidgetContainerItem");
    qmlRegisterUncreatableType<WidgetContainerItemAttached>(containerUri, 1, 0, "WidgetContainer", "For WidgetContainer attached prop.");
}

WidgetQuickItem *WidgetQuickItem::loadWidgetItem(Widget *widget, QQmlContext *pc)
{
    if (!widget) {
        return nullptr;
    }

    if (WidgetQuickItemPrivate::s_widgetItems.contains(widget)) {
        return WidgetQuickItemPrivate::s_widgetItems.value(widget);
    }

    std::call_once(flag, [] {
        WidgetQuickItem::registerTypes();
    });

    // 加载翻译文件
    WidgetQuickItemPrivate::installTranslator(widget);

    auto itemEngine = new WidgetItemEngine(widget, pc);
    itemEngine->loadMainItem();

    const auto qmlComponent = itemEngine->component();
    if (qmlComponent && qmlComponent->isError()) {
        widget->setUiError(itemEngine->component()->errorString());
        delete itemEngine;
        return nullptr;
    }

    auto mainItem = itemEngine->mainItem();
    if (!mainItem) {
        delete itemEngine;
        return nullptr;
    }

    // 类型判断
    WidgetQuickItem *item = nullptr;
    if (widget->type() == Types::Widget) {
        item = WidgetQuickItemPrivate::toWidgetItem(widget, mainItem);

    } else if (widget->type() == Types::Container) {
        item = WidgetQuickItemPrivate::toContainerItem(widget, mainItem);
    }

    if (!item) {
        delete mainItem;
        delete itemEngine;
        return nullptr;
    }

    // cache
    WidgetQuickItemPrivate::s_widgetItems.insert(widget, item);

    itemEngine->setParent(item);
    itemEngine->initContextProperty();
    item->d->itemEngine = itemEngine;

    const QString wid = widget->id();
    QObject::connect(widget, &Widget::aboutToDeleted, widget, [widget, wid]() {
        // TODO: 对s_widgetItems加个锁？
        delete WidgetQuickItemPrivate::s_widgetItems.take(widget);
        WidgetQuickItemPrivate::unInstallTranslator(wid);
    });

    return item;
}

void WidgetQuickItem::classBegin()
{
    QQuickItem::classBegin();

    // context
    auto context = qobject_cast<WidgetItemContext *>(QQmlEngine::contextForObject(this)->parentContext());
    if (context) {
        d->widget = context->widget();
    }
}

Widget *WidgetQuickItem::widget() const
{
    return d->widget;
}

QQmlContext *WidgetQuickItem::context() const
{
    return d->itemEngine->rootContext();
}

QQuickWindow* WidgetQuickItem::widgetWindow() const
{
    return QQuickItem::window();
}

bool WidgetQuickItem::adjustingLayout() const
{
    return d->adjustingLayout;
}

void WidgetQuickItem::setAdjustingLayout(bool adjustingLayout)
{
    if (d->adjustingLayout == adjustingLayout) {
        return;
    }

    d->adjustingLayout = adjustingLayout;
    Q_EMIT adjustingLayoutChanged();
}

// ====== WidgetItemPrivate ====== //
class WidgetItemPrivate
{
public:
    // Widget *widget{nullptr};
};

WidgetItem::WidgetItem(QQuickItem *parent) : WidgetQuickItem(parent), d(new WidgetItemPrivate)
{

}

WidgetItem::~WidgetItem()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

}
