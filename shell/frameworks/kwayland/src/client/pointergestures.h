/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_POINTERGESTURES_H
#define KWAYLAND_CLIENT_POINTERGESTURES_H

#include <QObject>
#include <QPointer>

#include "KWayland/Client/kwaylandclient_export.h"

struct zwp_pointer_gestures_v1;
struct zwp_pointer_gesture_swipe_v1;
struct zwp_pointer_gesture_pinch_v1;

class QSizeF;

namespace KWayland
{
namespace Client
{
class EventQueue;
class PointerPinchGesture;
class Pointer;
class Surface;
class PointerSwipeGesture;

/**
 * @short Wrapper for the zwp_pointer_gestures_v1 interface.
 *
 * This class provides a convenient wrapper for the zwp_pointer_gestures_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the PointerGestures interface:
 * @code
 * PointerGestures *c = registry->createPointerGestures(name, version);
 * @endcode
 *
 * This creates the PointerGestures and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * PointerGestures *c = new PointerGestures;
 * c->setup(registry->bindPointerGestures(name, version));
 * @endcode
 *
 * The PointerGestures can be used as a drop-in replacement for any zwp_pointer_gestures_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see PointerSwipeGesture
 * @see PointerPinchGesture
 * @since 5.29
 **/
class KWAYLANDCLIENT_EXPORT PointerGestures : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new PointerGestures.
     * Note: after constructing the PointerGestures it is not yet valid and one needs
     * to call setup. In order to get a ready to use PointerGestures prefer using
     * Registry::createPointerGestures.
     **/
    explicit PointerGestures(QObject *parent = nullptr);
    ~PointerGestures() override;

    /**
     * Setup this PointerGestures to manage the @p pointergestures.
     * When using Registry::createPointerGestures there is no need to call this
     * method.
     **/
    void setup(zwp_pointer_gestures_v1 *pointergestures);
    /**
     * @returns @c true if managing a zwp_pointer_gestures_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_pointer_gestures_v1 interface.
     * After the interface has been released the PointerGestures instance is no
     * longer valid and can be setup with another zwp_pointer_gestures_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this PointerGestures.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_pointer_gestures_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * PointerGestures gets destroyed.
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this PointerGestures.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this PointerGestures.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a PointerSwipeGesture for the given @p pointer with the @p parent.
     **/
    PointerSwipeGesture *createSwipeGesture(Pointer *pointer, QObject *parent = nullptr);

    /**
     * Creates a PointerPinchGesture for the given @p pointer with the @p parent.
     **/
    PointerPinchGesture *createPinchGesture(Pointer *pointer, QObject *parent = nullptr);

    operator zwp_pointer_gestures_v1 *();
    operator zwp_pointer_gestures_v1 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the PointerGestures got created by
     * Registry::createPointerGestures
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * This class is a wrapper for the zwp_pointer_gesture_swipe_v1 protocol.
 *
 * A PointerSwipeGesture object notifies a client about a multi-finger swipe
 * gesture detected on an indirect input device such as a touchpad.
 * The gesture is usually initiated by multiple fingers moving in the
 * same direction but once initiated the direction may change.
 * The precise conditions of when such a gesture is detected are
 * implementation-dependent.
 *
 * A gesture consists of three stages: begin, update (optional) and end.
 * There cannot be multiple simultaneous pinch or swipe gestures on the
 * same pointer/seat, how compositors prevent these situations is
 * implementation-dependent.
 *
 * A gesture may be cancelled by the compositor or the hardware.
 * Clients should not consider performing permanent or irreversible
 * actions until the end of a gesture has been received.
 *
 * @see PointerGestures
 * @see PointerPinchGesture
 * @since 5.29
 **/
class KWAYLANDCLIENT_EXPORT PointerSwipeGesture : public QObject
{
    Q_OBJECT
public:
    ~PointerSwipeGesture() override;

    /**
     * Setup this PointerSwipeGesture to manage the @p pointerswipegesture.
     * When using PointerGestures::createPointerSwipeGesture there is no need to call this
     * method.
     **/
    void setup(zwp_pointer_gesture_swipe_v1 *pointerswipegesture);
    /**
     * @returns @c true if managing a zwp_pointer_gesture_swipe_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_pointer_gesture_swipe_v1 interface.
     * After the interface has been released the PointerSwipeGesture instance is no
     * longer valid and can be setup with another zwp_pointer_gesture_swipe_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this PointerSwipeGesture.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_pointer_gesture_swipe_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, pointerswipegesture, &PointerSwipeGesture::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * The number of fingers taking part in this gesture.
     * If no gesture is in progress @c 0 is returned.
     **/
    quint32 fingerCount() const;

    /**
     * The Surface on which this gesture is performed.
     * If no gesture is in progress the returned pointer is null.
     **/
    QPointer<Surface> surface() const;

    operator zwp_pointer_gesture_swipe_v1 *();
    operator zwp_pointer_gesture_swipe_v1 *() const;

Q_SIGNALS:
    /**
     * A gesture got started.
     * @param serial Unique serial for this start gesture event.
     * @param time Timestamp in milliseconds granularity
     * @see updated
     * @see ended
     * @see cancelled
     **/
    void started(quint32 serial, quint32 time);

    /**
     * A gesture got updated.
     * @param delta relative coordinates of the logical center of the gesture compared to the previous event
     * @param time Timestamp in milliseconds granularity
     * @see started
     * @see ended
     * @see cancelled
     **/
    void updated(const QSizeF &delta, quint32 time);

    /**
     * A gesture ended.
     *
     * @param serial Unique serial for this end gesture event.
     * @param time Timestamp in milliseconds granularity
     * @see started
     * @see updated
     * @see cancelled
     **/
    void ended(quint32 serial, quint32 time);

    /**
     * A gesture got cancelled by the Wayland compositor.
     *
     * @param serial Unique serial for this cancel gesture event.
     * @param time Timestamp in milliseconds granularity
     * @see started
     * @see updated
     * @see ended
     **/
    void cancelled(quint32 serial, quint32 time);

private:
    friend class PointerGestures;
    explicit PointerSwipeGesture(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

/**
 * This class is a wrapper for the zwp_pointer_gesture_pinch_v1 protocol.
 *
 * A PointerPinchGesture object notifies a client about a multi-finger pinch
 * gesture detected on an indirect input device such as a touchpad.
 * The gesture is usually initiated by multiple fingers moving towards
 * each other or away from each other, or by two or more fingers rotating
 * around a logical center of gravity. The precise conditions of when
 * such a gesture is detected are implementation-dependent.
 *
 * A gesture consists of three stages: begin, update (optional) and end.
 * There cannot be multiple simultaneous pinch or swipe gestures on the
 * same pointer/seat, how compositors prevent these situations is
 * implementation-dependent.
 *
 * A gesture may be cancelled by the compositor or the hardware.
 * Clients should not consider performing permanent or irreversible
 * actions until the end of a gesture has been received.
 *
 * @see PointerGestures
 * @see PointerSwipeGesture
 * @since 5.29
 **/
class KWAYLANDCLIENT_EXPORT PointerPinchGesture : public QObject
{
    Q_OBJECT
public:
    ~PointerPinchGesture() override;

    /**
     * Setup this PointerPinchGesture to manage the @p pointerpinchgesture.
     * When using PointerGestures::createPointerPinchGesture there is no need to call this
     * method.
     **/
    void setup(zwp_pointer_gesture_pinch_v1 *pointerpinchgesture);
    /**
     * @returns @c true if managing a zwp_pointer_gesture_pinch_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_pointer_gesture_pinch_v1 interface.
     * After the interface has been released the PointerPinchGesture instance is no
     * longer valid and can be setup with another zwp_pointer_gesture_pinch_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this PointerPinchGesture.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_pointer_gesture_pinch_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, pointerpinchgesture, &PointerPinchGesture::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * The number of fingers taking part in this gesture.
     * If no gesture is in progress @c 0 is returned.
     **/
    quint32 fingerCount() const;

    /**
     * The Surface on which this gesture is performed.
     * If no gesture is in progress the returned pointer is null.
     **/
    QPointer<Surface> surface() const;

    operator zwp_pointer_gesture_pinch_v1 *();
    operator zwp_pointer_gesture_pinch_v1 *() const;

Q_SIGNALS:
    /**
     * A gesture got started.
     * @param serial Unique serial for this start gesture event.
     * @param time Timestamp in milliseconds granularity
     * @see updated
     * @see ended
     * @see cancelled
     **/
    void started(quint32 serial, quint32 time);

    /**
     * A gesture got updated.
     * @param delta relative coordinates of the logical center of the gesture compared to the previous event
     * @param scale an absolute scale compared to the start
     * @param rotation relative angle in degrees clockwise compared to the previous start or update event.
     * @param time Timestamp in milliseconds granularity
     * @see started
     * @see ended
     * @see cancelled
     **/
    void updated(const QSizeF &delta, qreal scale, qreal rotation, quint32 time);

    /**
     * A gesture ended.
     *
     * @param serial Unique serial for this end gesture event.
     * @param time Timestamp in milliseconds granularity
     * @see started
     * @see updated
     * @see cancelled
     **/
    void ended(quint32 serial, quint32 time);

    /**
     * A gesture got cancelled by the Wayland compositor.
     *
     * @param serial Unique serial for this cancel gesture event.
     * @param time Timestamp in milliseconds granularity
     * @see started
     * @see updated
     * @see ended
     **/
    void cancelled(quint32 serial, quint32 time);

private:
    friend class PointerGestures;
    explicit PointerPinchGesture(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
