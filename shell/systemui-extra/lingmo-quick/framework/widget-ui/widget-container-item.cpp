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

#include "widget-container-item.h"
#include "widget.h"
#include "widget-ui/widget-item.h"
#include "widget-container.h"
#include "shared-engine-component.h"
#include "widget-item-context.h"
#include "widget-interface.h"

#include <mutex>
#include <QMenu>
#include <QEvent>
#include <QMouseEvent>
#include <QQuickWindow>
#include <QTimer>

std::once_flag containerOnceFlag;

namespace LingmoUIQuick {

// WidgetContainerItemPrivate
class WidgetContainerItemPrivate
{
public:
    std::unique_ptr<QMenu> contextMenu;
    WidgetContainer *container {nullptr};
    SharedEngineComponent *component {nullptr};

    WidgetItemModel *itemModel {nullptr};
};

// ====== WidgetContainerItem ====== //
WidgetContainerItem::WidgetContainerItem(QQuickItem *parent) : WidgetQuickItem(parent), d(new WidgetContainerItemPrivate)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    d->itemModel = new WidgetItemModel(this);
}

WidgetContainerItem::~WidgetContainerItem()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

WidgetContainer *WidgetContainerItem::container() const
{
    return d->container;
}

SharedEngineComponent *WidgetContainerItem::component() const
{
    return d->component;
}

LingmoUIQuick::WidgetItemModel *WidgetContainerItem::widgetItemModel() const
{
    return d->itemModel;
}

WidgetItem *WidgetContainerItem::widgetItemForWidget(Widget *widget) const
{
    if (!widget || widget->type() != Types::Widget) {
        return nullptr;
    }

    WidgetQuickItem *item = WidgetQuickItem::loadWidgetItem(widget);
    return qobject_cast<WidgetItem *>(item);
}

void WidgetContainerItem::classBegin()
{
    WidgetQuickItem::classBegin();

    QQmlContext *context = QQmlEngine::contextForObject(this)->parentContext();
    if (context) {
        if (auto c = context->contextProperty("container").value<WidgetContainer*>()) {
            setContainer(c);
        }
    }
}

void WidgetContainerItem::setContainer(WidgetContainer *container)
{
    d->container = container;
    if (container) {
        for (const auto &widget : container->widgets()) {
            onWidgetAdded(widget);
        }

        connect(container, &WidgetContainer::widgetAdded, this, &WidgetContainerItem::onWidgetAdded);
        connect(container, &WidgetContainer::widgetRemoved, this, &WidgetContainerItem::onWidgetRemoved);
    }
}

void WidgetContainerItem::onWidgetAdded(Widget *widget)
{
    WidgetQuickItem *widgetItem = WidgetQuickItem::loadWidgetItem(widget);
    connect(this, &WidgetItem::adjustingLayoutChanged, widgetItem, [&, widgetItem]() {
        widgetItem->setAdjustingLayout(adjustingLayout());
    });
    if (d->itemModel->insertWidgetItem(widgetItem)) {
        Q_EMIT widgetItemAdded(widgetItem);
    }
}

void WidgetContainerItem::onWidgetRemoved(Widget *widget)
{
    WidgetQuickItem *widgetItem = WidgetQuickItem::loadWidgetItem(widget);
    if (d->itemModel->removeWidgetItem(widgetItem)) {
        Q_EMIT widgetItemRemoved(widgetItem);
    }
}

bool WidgetContainerItem::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        switch (mouseEvent->button()) {
            case Qt::MiddleButton:
            case Qt::LeftButton: {
                // 执行 Action
                if (d->contextMenu) {
                    // 关闭菜单
                    d->contextMenu->close();
                }
                break;
            }
            case Qt::RightButton:
                // 显示菜单
                if (showContextMenu(mouseEvent)) {
                    //https://bugreports.qt.io/browse/QTBUG-59044
                    QTimer::singleShot(0, this, [this]() {
                        if (window() && window()->mouseGrabberItem()) {
                            window()->mouseGrabberItem()->ungrabMouse();
                        }
                    });
                    return true;
                }
                break;
            case Qt::BackButton:
            case Qt::ForwardButton:
                // 循环
                break;
            default:
                break;
        }
    }

    return WidgetQuickItem::event(event);
}

bool WidgetContainerItem::showContextMenu(QMouseEvent *mouseEvent)
{
    if (!container()) {
        return false;
    }

    if (d->contextMenu) {
        // 连续两次右键关闭右键菜单
        if (d->contextMenu->isVisible()) {
            d->contextMenu->close();
            return false;
        }

        d->contextMenu->close();
        for (const auto &action : d->contextMenu->actions()) {
            if (action->menu()) {
                // 断开菜单窗口之间的信号
                action->menu()->disconnect(d->contextMenu.get());
            }
        }
        d->contextMenu->clear();

    } else {
        d->contextMenu.reset(new QMenu);
        d->contextMenu->setAttribute(Qt::WA_DeleteOnClose, false);
        connect(d->contextMenu.get(), &QMenu::aboutToHide, this, [&](){
            d->container->setActive(false);
        });
    }

    // 1.添加widget的Action
    QList<QAction *> widgetActions;
    QPointF pointF = mouseEvent->pos();
    for (const auto &widgetItem : qAsConst(d->itemModel->m_widgetItems)) {
        const Widget *widget = widgetItem->widget();
        QRectF widgetRect = widgetItem->mapRectToItem(this, {widgetItem->position(), widgetItem->size()});
        if (widget && widgetRect.contains(pointF)) {
            widgetActions << widget->actions();
            break;
        }
    }

    // 2.添加Container的Action,一般代表程序窗口的某些操作
    // 插件存在Action那么就不显示Container的菜单
    //插件通过WidgetInterface加载的action会被加到最后
    if (widgetActions.isEmpty()) {
        QList<QAction *> widgetPluginActions;
        for(auto w : d->container->widgets()) {
            if (w && w->plugin()) {
                widgetPluginActions.append(w->plugin()->actions());
            }
        }
        container()->setAppendActions(widgetPluginActions);
        //如果有父container存在,由父container显示菜单
        if(container()->container()) {
            return false;
        }
        d->contextMenu->addActions(container()->actions());

    } else {
        d->contextMenu->addActions(widgetActions);
    }

    if (d->contextMenu->isEmpty()) {
        return false;
    }

    // 创建window
    d->contextMenu->winId();
    if (d->contextMenu->windowHandle()) {
        d->contextMenu->windowHandle()->setTransientParent(window());
    }

    for (const auto &action : d->contextMenu->actions()) {
        const QMenu *menu = action->menu();
        if (menu) {
            connect(menu, &QMenu::aboutToShow, d->contextMenu.get(), [this, menu] {
                if (menu->windowHandle()) {
                    menu->windowHandle()->setTransientParent(d->contextMenu->windowHandle());
                }
            });
        }
    }

    d->container->setActive(true);
    d->contextMenu->popup(mouseEvent->globalPos());
    return true;
}

// ====== WidgetContainerItemAttached ====== //
WidgetContainer *WidgetContainerItemAttached::qmlAttachedProperties(QObject *object)
{
    auto containerItem = qobject_cast<WidgetContainerItem *>(object);
    if (containerItem) {
        return containerItem->container();
    }

    QQmlContext *context = QQmlEngine::contextForObject(object);
    while (context) {
        context = context->parentContext();
        if (auto c = qobject_cast<WidgetItemContext *>(context)) {
            return qobject_cast<WidgetContainer *>(c->widget());
        }
    }

    return nullptr;
}

// ====== WidgetItemModel ======
WidgetItemModel::WidgetItemModel(QObject *parent) : QAbstractListModel(parent)
{

}

int WidgetItemModel::rowCount(const QModelIndex &parent) const
{
    return m_widgetItems.size();
}

int WidgetItemModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant WidgetItemModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    if (role == WidgetItemRole) {
        return QVariant::fromValue(m_widgetItems.at(index.row()));
    }

    return {};
}

QHash<int, QByteArray> WidgetItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(WidgetItemRole, "widgetItem");
    return roles;
}

bool WidgetItemModel::insertWidgetItem(WidgetQuickItem *item)
{
    if (!item || m_widgetItems.contains(item)) {
        return false;
    }

    beginInsertRows(QModelIndex(), m_widgetItems.size(), m_widgetItems.size());
    m_widgetItems.append(item);
    endInsertRows();

    return true;
}

bool WidgetItemModel::removeWidgetItem(WidgetQuickItem *item)
{
    if (!item || !m_widgetItems.contains(item)) {
        return false;
    }

    int idx = m_widgetItems.indexOf(item);
    beginRemoveRows(QModelIndex(), idx, idx);
    m_widgetItems.takeAt(idx);
    endRemoveRows();

    return true;
}

} // LingmoUIQuick
