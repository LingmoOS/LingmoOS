/*
    SPDX-FileCopyrightText: 2011 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mouseeventlistener.h"

#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QQuickWindow>
#include <QScreen>
#include <QStyleHints>
#include <QTimer>

MouseEventListener::MouseEventListener(QQuickItem *parent)
    : QQuickItem(parent)
    , m_pressed(false)
    , m_pressAndHoldEvent(nullptr)
    , m_lastEvent(nullptr)
    , m_acceptedButtons(Qt::LeftButton)
{
    m_pressAndHoldTimer = new QTimer(this);
    m_pressAndHoldTimer->setSingleShot(true);
    connect(m_pressAndHoldTimer, &QTimer::timeout, this, &MouseEventListener::handlePressAndHold);
    setFiltersChildMouseEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton | Qt::XButton1 | Qt::XButton2);
}

MouseEventListener::~MouseEventListener()
{
}

Qt::MouseButtons MouseEventListener::acceptedButtons() const
{
    return m_acceptedButtons;
}

Qt::CursorShape MouseEventListener::cursorShape() const
{
    return cursor().shape();
}

void MouseEventListener::setCursorShape(Qt::CursorShape shape)
{
    if (cursor().shape() == shape) {
        return;
    }

    setCursor(shape);

    Q_EMIT cursorShapeChanged();
}

void MouseEventListener::setAcceptedButtons(Qt::MouseButtons buttons)
{
    if (buttons == m_acceptedButtons) {
        return;
    }

    m_acceptedButtons = buttons;
    Q_EMIT acceptedButtonsChanged();
}

void MouseEventListener::setHoverEnabled(bool enable)
{
    if (enable == acceptHoverEvents()) {
        return;
    }

    setAcceptHoverEvents(enable);
    Q_EMIT hoverEnabledChanged(enable);
}

bool MouseEventListener::hoverEnabled() const
{
    return acceptHoverEvents();
}

bool MouseEventListener::isPressed() const
{
    return m_pressed;
}

void MouseEventListener::hoverEnterEvent(QHoverEvent *event)
{
    Q_UNUSED(event);

    m_containsMouse = true;
    if (!m_childContainsMouse) {
        Q_EMIT containsMouseChanged(true);
    }
}

void MouseEventListener::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event);

    m_containsMouse = false;
    if (!m_childContainsMouse) {
        Q_EMIT containsMouseChanged(false);
    }
}

void MouseEventListener::hoverMoveEvent(QHoverEvent *event)
{
    if (m_lastEvent == event) {
        return;
    }

    QQuickWindow *w = window();
    QPoint screenPos;
    if (w) {
        screenPos = w->mapToGlobal(event->position()).toPoint();
    }

    KDeclarativeMouseEvent dme(event->position().x(),
                               event->position().y(),
                               screenPos.x(),
                               screenPos.y(),
                               Qt::NoButton,
                               Qt::NoButton,
                               event->modifiers(),
                               nullptr,
                               Qt::MouseEventNotSynthesized);
    Q_EMIT positionChanged(&dme);
}

bool MouseEventListener::containsMouse() const
{
    return m_containsMouse || m_childContainsMouse;
}

void MouseEventListener::mousePressEvent(QMouseEvent *me)
{
    if (m_lastEvent == me || !(me->buttons() & m_acceptedButtons)) {
        me->setAccepted(false);
        return;
    }

    // FIXME: when a popup window is visible: a click anywhere hides it: but the old qquickitem will continue to think it's under the mouse
    // doesn't seem to be any good way to properly reset this.
    // this msolution will still caused a missed click after the popup is gone, but gets the situation unblocked.
    QPoint viewPosition;
    if (window()) {
        viewPosition = window()->position();
    }

    if (!QRectF(mapToScene(QPoint(0, 0)) + viewPosition, QSizeF(width(), height())).contains(me->globalPosition())) {
        me->ignore();
        return;
    }
    m_buttonDownPos = me->globalPosition();

    KDeclarativeMouseEvent dme(me->pos().x(),
                               me->pos().y(),
                               me->globalPosition().x(),
                               me->globalPosition().y(),
                               me->button(),
                               me->buttons(),
                               me->modifiers(),
                               screenForGlobalPos(me->globalPosition()),
                               me->source());
    if (!m_pressAndHoldEvent) {
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(me->pos().x(),
                                                         me->pos().y(),
                                                         me->globalPosition().x(),
                                                         me->globalPosition().y(),
                                                         me->button(),
                                                         me->buttons(),
                                                         me->modifiers(),
                                                         screenForGlobalPos(me->globalPosition()),
                                                         me->source());
    }

    m_pressed = true;
    Q_EMIT pressed(&dme);
    Q_EMIT pressedChanged();

    if (dme.isAccepted()) {
        me->setAccepted(true);
        return;
    }

    m_pressAndHoldTimer->start(QGuiApplication::styleHints()->mousePressAndHoldInterval());
}

void MouseEventListener::mouseMoveEvent(QMouseEvent *me)
{
    if (m_lastEvent == me || !(me->buttons() & m_acceptedButtons)) {
        me->setAccepted(false);
        return;
    }

    if (QPointF(me->globalPosition() - m_buttonDownPos).manhattanLength() > QGuiApplication::styleHints()->startDragDistance()
        && m_pressAndHoldTimer->isActive()) {
        m_pressAndHoldTimer->stop();
    }

    KDeclarativeMouseEvent dme(me->pos().x(),
                               me->pos().y(),
                               me->globalPosition().x(),
                               me->globalPosition().y(),
                               me->button(),
                               me->buttons(),
                               me->modifiers(),
                               screenForGlobalPos(me->globalPosition()),
                               me->source());
    Q_EMIT positionChanged(&dme);

    if (dme.isAccepted()) {
        me->setAccepted(true);
    }
}

void MouseEventListener::mouseReleaseEvent(QMouseEvent *me)
{
    if (m_lastEvent == me) {
        me->setAccepted(false);
        return;
    }

    KDeclarativeMouseEvent dme(me->pos().x(),
                               me->pos().y(),
                               me->globalPosition().x(),
                               me->globalPosition().y(),
                               me->button(),
                               me->buttons(),
                               me->modifiers(),
                               screenForGlobalPos(me->globalPosition()),
                               me->source());
    m_pressed = false;
    Q_EMIT released(&dme);
    Q_EMIT pressedChanged();

    if (boundingRect().contains(me->pos()) && m_pressAndHoldTimer->isActive()) {
        Q_EMIT clicked(&dme);
        m_pressAndHoldTimer->stop();
    }

    if (dme.isAccepted()) {
        me->setAccepted(true);
    }
}

void MouseEventListener::wheelEvent(QWheelEvent *we)
{
    if (m_lastEvent == we) {
        return;
    }

    KDeclarativeWheelEvent dwe(we->position().toPoint(),
                               we->globalPosition().toPoint(),
                               we->angleDelta(),
                               we->buttons(),
                               we->modifiers(),
                               Qt::Vertical /* HACK, deprecated, remove */);
    Q_EMIT wheelMoved(&dwe);
}

void MouseEventListener::handlePressAndHold()
{
    if (m_pressed) {
        Q_EMIT pressAndHold(m_pressAndHoldEvent);

        delete m_pressAndHoldEvent;
        m_pressAndHoldEvent = nullptr;
    }
}

bool MouseEventListener::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    // don't filter other mouseeventlisteners
    if (qobject_cast<MouseEventListener *>(item)) {
        return false;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        m_lastEvent = event;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        if (!(me->buttons() & m_acceptedButtons)) {
            break;
        }

        // the parent will receive events in its own coordinates
        const QPointF myPos = mapFromScene(me->scenePosition());

        KDeclarativeMouseEvent dme(myPos.x(),
                                   myPos.y(),
                                   me->globalPosition().x(),
                                   me->globalPosition().y(),
                                   me->button(),
                                   me->buttons(),
                                   me->modifiers(),
                                   screenForGlobalPos(me->globalPosition()),
                                   me->source());
        delete m_pressAndHoldEvent;
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(),
                                                         myPos.y(),
                                                         me->globalPosition().x(),
                                                         me->globalPosition().y(),
                                                         me->button(),
                                                         me->buttons(),
                                                         me->modifiers(),
                                                         screenForGlobalPos(me->globalPosition()),
                                                         me->source());

        // qDebug() << "pressed in sceneEventFilter";
        m_buttonDownPos = me->globalPosition();
        m_pressed = true;
        Q_EMIT pressed(&dme);
        Q_EMIT pressedChanged();

        if (dme.isAccepted()) {
            return true;
        }

        m_pressAndHoldTimer->start(QGuiApplication::styleHints()->mousePressAndHoldInterval());

        break;
    }
    case QEvent::HoverMove: {
        if (!acceptHoverEvents()) {
            break;
        }
        m_lastEvent = event;
        QHoverEvent *he = static_cast<QHoverEvent *>(event);
        const QPointF myPos = item->mapToItem(this, he->position());

        QQuickWindow *w = window();
        QPoint screenPos;
        if (w) {
            screenPos = w->mapToGlobal(myPos.toPoint());
        }

        KDeclarativeMouseEvent
            dme(myPos.x(), myPos.y(), screenPos.x(), screenPos.y(), Qt::NoButton, Qt::NoButton, he->modifiers(), nullptr, Qt::MouseEventNotSynthesized);
        // qDebug() << "positionChanged..." << dme.x() << dme.y();
        Q_EMIT positionChanged(&dme);

        if (dme.isAccepted()) {
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        m_lastEvent = event;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (!(me->buttons() & m_acceptedButtons)) {
            break;
        }

        const QPointF myPos = mapFromScene(me->scenePosition());
        KDeclarativeMouseEvent dme(myPos.x(),
                                   myPos.y(),
                                   me->globalPosition().x(),
                                   me->globalPosition().y(),
                                   me->button(),
                                   me->buttons(),
                                   me->modifiers(),
                                   screenForGlobalPos(me->globalPosition()),
                                   me->source());
        // qDebug() << "positionChanged..." << dme.x() << dme.y();

        // stop the pressandhold if mouse moved enough
        if (QPointF(me->globalPosition() - m_buttonDownPos).manhattanLength() > QGuiApplication::styleHints()->startDragDistance()
            && m_pressAndHoldTimer->isActive()) {
            m_pressAndHoldTimer->stop();

            // if the mouse moves and we are waiting to emit a press and hold event, update the coordinates
            // as there is no update function, delete the old event and create a new one
        } else if (m_pressAndHoldEvent) {
            delete m_pressAndHoldEvent;
            m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(),
                                                             myPos.y(),
                                                             me->globalPosition().x(),
                                                             me->globalPosition().y(),
                                                             me->button(),
                                                             me->buttons(),
                                                             me->modifiers(),
                                                             screenForGlobalPos(me->globalPosition()),
                                                             me->source());
        }
        Q_EMIT positionChanged(&dme);

        if (dme.isAccepted()) {
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        m_lastEvent = event;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        const QPointF myPos = mapFromScene(me->scenePosition());
        KDeclarativeMouseEvent dme(myPos.x(),
                                   myPos.y(),
                                   me->globalPosition().x(),
                                   me->globalPosition().y(),
                                   me->button(),
                                   me->buttons(),
                                   me->modifiers(),
                                   screenForGlobalPos(me->globalPosition()),
                                   me->source());
        m_pressed = false;

        Q_EMIT released(&dme);
        Q_EMIT pressedChanged();

        if (QPointF(me->globalPosition() - m_buttonDownPos).manhattanLength() <= QGuiApplication::styleHints()->startDragDistance()
            && m_pressAndHoldTimer->isActive()) {
            Q_EMIT clicked(&dme);
            m_pressAndHoldTimer->stop();
        }

        if (dme.isAccepted()) {
            return true;
        }
        break;
    }
    case QEvent::UngrabMouse: {
        m_lastEvent = event;
        handleUngrab();
        break;
    }
    case QEvent::Wheel: {
        m_lastEvent = event;
        QWheelEvent *we = static_cast<QWheelEvent *>(event);
        KDeclarativeWheelEvent dwe(we->position().toPoint(),
                                   we->globalPosition().toPoint(),
                                   we->angleDelta(),
                                   we->buttons(),
                                   we->modifiers(),
                                   Qt::Vertical /* HACK, deprecated, remove */);
        Q_EMIT wheelMoved(&dwe);
        break;
    }
    case QEvent::HoverEnter: {
        m_childContainsMouse = true;
        if (!m_containsMouse) {
            Q_EMIT containsMouseChanged(true);
        }
        break;
    }
    case QEvent::HoverLeave: {
        m_childContainsMouse = false;
        if (!m_containsMouse) {
            Q_EMIT containsMouseChanged(false);
        }
        break;
    }
    default:
        break;
    }

    return QQuickItem::childMouseEventFilter(item, event);
    //    return false;
}

QScreen *MouseEventListener::screenForGlobalPos(const QPointF &globalPos)
{
    const auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (screen->geometry().contains(globalPos.toPoint())) {
            return screen;
        }
    }
    return nullptr;
}

void MouseEventListener::mouseUngrabEvent()
{
    handleUngrab();

    QQuickItem::mouseUngrabEvent();
}

void MouseEventListener::touchUngrabEvent()
{
    handleUngrab();

    QQuickItem::touchUngrabEvent();
}

void MouseEventListener::handleUngrab()
{
    if (m_pressed) {
        m_pressAndHoldTimer->stop();

        m_pressed = false;
        Q_EMIT pressedChanged();

        Q_EMIT canceled();
    }
}

#include "moc_mouseeventlistener.cpp"
