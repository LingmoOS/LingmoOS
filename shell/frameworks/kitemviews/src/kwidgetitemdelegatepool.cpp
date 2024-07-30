/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwidgetitemdelegatepool_p.h"

#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QApplication>
#include <QHash>
#include <QInputEvent>
#include <QList>
#include <QMetaMethod>
#include <QWidget>
#include <qobjectdefs.h>

#include "kwidgetitemdelegate.h"
#include "kwidgetitemdelegate_p.h"
#include <kitemviews_debug.h>

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KWidgetItemDelegateEventListener : public QObject
{
public:
    KWidgetItemDelegateEventListener(KWidgetItemDelegatePoolPrivate *poolPrivate, QObject *parent = nullptr)
        : QObject(parent)
        , poolPrivate(poolPrivate)
    {
    }

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    KWidgetItemDelegatePoolPrivate *const poolPrivate;
};

KWidgetItemDelegatePoolPrivate::KWidgetItemDelegatePoolPrivate(KWidgetItemDelegate *d)
    : delegate(d)
    , eventListener(new KWidgetItemDelegateEventListener(this))
{
}

KWidgetItemDelegatePool::KWidgetItemDelegatePool(KWidgetItemDelegate *delegate)
    : d(new KWidgetItemDelegatePoolPrivate(delegate))
{
}

KWidgetItemDelegatePool::~KWidgetItemDelegatePool()
{
    delete d->eventListener;
    delete d;
}

QList<QWidget *>
KWidgetItemDelegatePool::findWidgets(const QPersistentModelIndex &idx, const QStyleOptionViewItem &option, UpdateWidgetsEnum updateWidgets) const
{
    QList<QWidget *> result;

    if (!idx.isValid()) {
        return result;
    }

    QModelIndex index;
    if (const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel *>(idx.model())) {
        index = proxyModel->mapToSource(idx);
    } else {
        index = idx;
    }

    if (!index.isValid()) {
        return result;
    }

    if (d->usedWidgets.contains(index)) {
        result = d->usedWidgets[index];
    } else {
        result = d->delegate->createItemWidgets(index);
        d->usedWidgets[index] = result;
        for (QWidget *widget : std::as_const(result)) {
            d->widgetInIndex[widget] = index;
            widget->setParent(d->delegate->d->itemView->viewport());
            widget->installEventFilter(d->eventListener);
            widget->setVisible(true);
        }
    }

    if (updateWidgets == UpdateWidgets) {
        for (QWidget *widget : std::as_const(result)) {
            widget->setVisible(true);
        }

        d->delegate->updateItemWidgets(result, option, idx);

        for (QWidget *widget : std::as_const(result)) {
            widget->move(widget->x() + option.rect.left(), widget->y() + option.rect.top());
        }
    }

    return result;
}

QList<QWidget *> KWidgetItemDelegatePool::invalidIndexesWidgets() const
{
    QList<QWidget *> result;
    QHashIterator<QWidget *, QPersistentModelIndex> i(d->widgetInIndex);
    while (i.hasNext()) {
        i.next();
        const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel *>(d->delegate->d->model);
        QModelIndex index;
        if (proxyModel) {
            index = proxyModel->mapFromSource(i.value());
        } else {
            index = i.value();
        }
        if (!index.isValid()) {
            result << i.key();
        }
    }
    return result;
}

void KWidgetItemDelegatePool::fullClear()
{
    d->clearing = true;
    qDeleteAll(d->widgetInIndex.keys());
    d->clearing = false;
    d->usedWidgets.clear();
    d->widgetInIndex.clear();
}

bool KWidgetItemDelegateEventListener::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *widget = static_cast<QWidget *>(watched);

    if (event->type() == QEvent::Destroy && !poolPrivate->clearing) {
        qCWarning(KITEMVIEWS_LOG) << "User of KWidgetItemDelegate should not delete widgets created by createItemWidgets!";
        // assume the application has kept a list of widgets and tries to delete them manually
        // they have been reparented to the view in any case, so no leaking occurs
        poolPrivate->widgetInIndex.remove(widget);
    }
    if (dynamic_cast<QInputEvent *>(event) && !poolPrivate->delegate->blockedEventTypes(widget).contains(event->type())) {
        QWidget *viewport = poolPrivate->delegate->d->itemView->viewport();
        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QMouseEvent evt(event->type(),
                            mouseEvent->position(),
                            viewport->mapFromGlobal(mouseEvent->globalPosition()),
                            mouseEvent->button(),
                            mouseEvent->buttons(),
                            mouseEvent->modifiers());
            QApplication::sendEvent(viewport, &evt);
        } break;
        case QEvent::Wheel: {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            QWheelEvent evt(viewport->mapFromGlobal(wheelEvent->position().toPoint()),
                            viewport->mapFromGlobal(wheelEvent->globalPosition().toPoint()),
                            wheelEvent->pixelDelta(),
                            wheelEvent->angleDelta(),
                            wheelEvent->buttons(),
                            wheelEvent->modifiers(),
                            wheelEvent->phase(),
                            wheelEvent->inverted(),
                            wheelEvent->source());
            QApplication::sendEvent(viewport, &evt);
        } break;
        case QEvent::TabletMove:
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletEnterProximity:
        case QEvent::TabletLeaveProximity: {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);
            QTabletEvent evt(event->type(),
                             tabletEvent->pointingDevice(),
                             viewport->mapFromGlobal(tabletEvent->globalPosition()),
                             tabletEvent->globalPosition(),
                             tabletEvent->pressure(),
                             tabletEvent->xTilt(),
                             tabletEvent->yTilt(),
                             tabletEvent->tangentialPressure(),
                             tabletEvent->rotation(),
                             tabletEvent->z(),
                             tabletEvent->modifiers(),
                             tabletEvent->button(),
                             tabletEvent->buttons());
            QApplication::sendEvent(viewport, &evt);
            break;
        }
        default:
            QApplication::sendEvent(viewport, event);
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}
//@endcond
