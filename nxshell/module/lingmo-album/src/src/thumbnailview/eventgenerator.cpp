/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "eventgenerator.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>

EventGenerator::EventGenerator(QObject *parent)
    : QObject(parent)
{
}

EventGenerator::~EventGenerator()
{
}

void EventGenerator::sendMouseEvent(QQuickItem *item,
                                    EventGenerator::MouseEvent type,
                                    int x,
                                    int y,
                                    int button,
                                    Qt::MouseButtons buttons,
                                    Qt::KeyboardModifiers modifiers)
{
    if (!item) {
        return;
    }

    QEvent::Type eventType;
    switch (type) {
    case MouseButtonPress:
        eventType = QEvent::MouseButtonPress;
        break;
    case MouseButtonRelease:
        eventType = QEvent::MouseButtonRelease;
        break;
    case MouseMove:
        eventType = QEvent::MouseMove;
        break;
    default:
        return;
    }
    QMouseEvent ev(eventType, QPointF(x, y), static_cast<Qt::MouseButton>(button), buttons, modifiers);

    QGuiApplication::sendEvent(item, &ev);
}

void EventGenerator::sendMouseEventRecursive(QQuickItem *parentItem,
                                             EventGenerator::MouseEvent type,
                                             int x,
                                             int y,
                                             int button,
                                             Qt::MouseButtons buttons,
                                             Qt::KeyboardModifiers modifiers)
{
    if (!parentItem) {
        return;
    }

    const QList<QQuickItem *> items = allChildItemsRecursive(parentItem);

    for (QQuickItem *item : items) {
        sendMouseEvent(item, type, x, y, button, buttons, modifiers);
    }
}

void EventGenerator::sendWheelEvent(QQuickItem *item,
                                    int x,
                                    int y,
                                    const QPoint &pixelDelta,
                                    const QPoint &angleDelta,
                                    Qt::MouseButtons buttons,
                                    Qt::KeyboardModifiers modifiers)
{
    if (!item || !item->window()) {
        return;
    }

    QPointF pos(x, y);
    QPointF globalPos(item->window()->mapToGlobal(item->mapToScene(pos).toPoint()));
    QWheelEvent ev(pos, globalPos, pixelDelta, angleDelta, buttons, modifiers, Qt::ScrollUpdate, false /*not inverted*/);
    QGuiApplication::sendEvent(item, &ev);
}

void EventGenerator::sendWheelEventRecursive(QQuickItem *parentItem,
                                             int x,
                                             int y,
                                             const QPoint &pixelDelta,
                                             const QPoint &angleDelta,
                                             Qt::MouseButtons buttons,
                                             Qt::KeyboardModifiers modifiers)
{
    if (!parentItem) {
        return;
    }

    const QList<QQuickItem *> items = allChildItemsRecursive(parentItem);

    for (QQuickItem *item : items) {
        sendWheelEvent(item, x, y, pixelDelta, angleDelta, buttons, modifiers);
    }
}

void EventGenerator::sendGrabEvent(QQuickItem *item, EventGenerator::GrabEvent type)
{
    if (!item) {
        return;
    }

    switch (type) {
    case GrabMouse:
        item->grabMouse();
        break;
    case UngrabMouse: {
        QEvent ev(QEvent::UngrabMouse);
        QGuiApplication::sendEvent(item, &ev);
        return;
    }
    default:
        return;
    }
}

void EventGenerator::sendGrabEventRecursive(QQuickItem *parentItem, EventGenerator::GrabEvent type)
{
    if (!parentItem) {
        return;
    }

    const QList<QQuickItem *> items = allChildItemsRecursive(parentItem);

    for (QQuickItem *item : items) {
        sendGrabEvent(item, type);
    }
}

QList<QQuickItem *> EventGenerator::allChildItemsRecursive(QQuickItem *parentItem)
{
    QList<QQuickItem *> itemList;

    const auto childsItems = parentItem->childItems();
    itemList.append(childsItems);

    for (QQuickItem *childItem : childsItems) {
        itemList.append(allChildItemsRecursive(childItem));
    }

    return itemList;
}
