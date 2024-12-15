/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EventGenerator_H
#define EventGenerator_H

#include <QObject>

class QQuickItem;

class EventGenerator : public QObject
{
    Q_OBJECT

public:
    enum MouseEvent {
        MouseButtonPress,
        MouseButtonRelease,
        MouseMove,
    };
    Q_ENUM(MouseEvent)

    enum GrabEvent {
        GrabMouse,
        UngrabMouse,
    };
    Q_ENUM(GrabEvent)

    EventGenerator(QObject *parent = nullptr);
    ~EventGenerator() override;

    /**
     * Send a mouse event of @type to the given @item
     */
    Q_INVOKABLE void
    sendMouseEvent(QQuickItem *item, EventGenerator::MouseEvent type, int x, int y, int button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

    /**
     * Send a mouse event of @type to the given @item, all its children and descendants
     */
    Q_INVOKABLE void sendMouseEventRecursive(QQuickItem *item,
                                             EventGenerator::MouseEvent type,
                                             int x,
                                             int y,
                                             int button,
                                             Qt::MouseButtons buttons,
                                             Qt::KeyboardModifiers modifiers);

    /**
     * Send a wheel event to the given @item
     *
     * @since 5.16
     */
    Q_INVOKABLE void sendWheelEvent(QQuickItem *item,
                                    int x,
                                    int y,
                                    const QPoint &pixelDelta,
                                    const QPoint &angleDelta,
                                    Qt::MouseButtons buttons,
                                    Qt::KeyboardModifiers modifiers);

    /**
     * Send a wheel event to the given @item, all its children and descendants
     *
     * @since 5.16
     */
    Q_INVOKABLE void sendWheelEventRecursive(QQuickItem *item,
                                             int x,
                                             int y,
                                             const QPoint &pixelDelta,
                                             const QPoint &angleDelta,
                                             Qt::MouseButtons buttons,
                                             Qt::KeyboardModifiers modifiers);

    /**
     * Send a mouse grab event of @type (grab or ungrab) to the given @item
     */
    Q_INVOKABLE void sendGrabEvent(QQuickItem *item, EventGenerator::GrabEvent type);

    /**
     * Send a mouse grab event of @type (grab or ungrab) to the given @item, all its children and descendants
     */
    Q_INVOKABLE void sendGrabEventRecursive(QQuickItem *item, EventGenerator::GrabEvent type);

private:
    static QList<QQuickItem *> allChildItemsRecursive(QQuickItem *parentItem);
};

#endif
