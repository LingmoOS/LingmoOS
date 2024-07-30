/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_RELATIVEPOINTER_H
#define KWAYLAND_CLIENT_RELATIVEPOINTER_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct zwp_relative_pointer_manager_v1;
struct zwp_relative_pointer_v1;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Pointer;
class RelativePointer;

/**
 * @short Wrapper for the zwp_relative_pointer_manager_v1 interface.
 *
 * This class provides a convenient wrapper for the zwp_relative_pointer_manager_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the RelativePointerManager interface:
 * @code
 * RelativePointerManager *c = registry->createRelativePointerManagerUnstableV1(name, version);
 * @endcode
 *
 * This creates the RelativePointerManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * RelativePointerManager *c = new RelativePointerManager;
 * c->setup(registry->RelativePointerManager(name, version));
 * @endcode
 *
 * The RelativePointerManager can be used as a drop-in replacement for any zwp_relative_pointer_manager_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.28
 **/
class KWAYLANDCLIENT_EXPORT RelativePointerManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new RelativePointerManager.
     * Note: after constructing the RelativePointerManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use RelativePointerManager prefer using
     * Registry::createRelativePointerManagerUnstableV1.
     **/
    explicit RelativePointerManager(QObject *parent = nullptr);
    ~RelativePointerManager() override;

    /**
     * Setup this RelativePointerManagerUnstableV1 to manage the @p relativepointermanagerunstablev1.
     * When using Registry::createRelativePointerManagerUnstableV1 there is no need to call this
     * method.
     **/
    void setup(zwp_relative_pointer_manager_v1 *relativepointermanagerunstablev1);
    /**
     * @returns @c true if managing a zwp_relative_pointer_manager_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_relative_pointer_manager_v1 interface.
     * After the interface has been released the RelativePointerManagerUnstableV1 instance is no
     * longer valid and can be setup with another zwp_relative_pointer_manager_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this RelativePointerManagerUnstableV1.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_relative_pointer_manager_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, relativepointermanagerunstablev1, &RelativePointerManagerUnstableV1::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this RelativePointerManagerUnstableV1.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this RelativePointerManagerUnstableV1.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a RelativePointer for the given @p pointer.
     **/
    RelativePointer *createRelativePointer(Pointer *pointer, QObject *parent = nullptr);

    operator zwp_relative_pointer_manager_v1 *();
    operator zwp_relative_pointer_manager_v1 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the RelativePointerManagerUnstableV1 got created by
     * Registry::createRelativePointerManagerUnstableV1
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the zwp_relative_pointer_v1 interface.
 *
 * The RelativePointer is an extension to the Pointer used for emitting
 * relative pointer events. It shares the same focus as Pointer of the same Seat
 * and will only emit events when it has focus.
 *
 * @since 5.28
 **/
class KWAYLANDCLIENT_EXPORT RelativePointer : public QObject
{
    Q_OBJECT
public:
    ~RelativePointer() override;

    /**
     * Setup this RelativePointerUnstableV1 to manage the @p relativepointerunstablev1.
     * When using RelativePointerManagerUnstableV1::createRelativePointerUnstableV1 there is no need to call this
     * method.
     **/
    void setup(zwp_relative_pointer_v1 *relativepointerunstablev1);
    /**
     * @returns @c true if managing a zwp_relative_pointer_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_relative_pointer_v1 interface.
     * After the interface has been released the RelativePointerUnstableV1 instance is no
     * longer valid and can be setup with another zwp_relative_pointer_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this RelativePointerUnstableV1.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_relative_pointer_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * RelativePointer gets destroyed.
     *
     * @see release
     **/
    void destroy();

    operator zwp_relative_pointer_v1 *();
    operator zwp_relative_pointer_v1 *() const;

Q_SIGNALS:
    /**
     * A relative motion event.
     *
     * A relative motion is in the same dimension as regular motion events,
     * except they do not represent an absolute position. For example,
     * moving a pointer from (x, y) to (x', y') would have the equivalent
     * relative motion (x' - x, y' - y). If a pointer motion caused the
     * absolute pointer position to be clipped by for example the edge of the
     * monitor, the relative motion is unaffected by the clipping and will
     * represent the unclipped motion.
     *
     * This signal also contains non-accelerated motion deltas (@p deltaNonAccelerated).
     * The non-accelerated delta is, when applicable, the regular pointer motion
     * delta as it was before having applied motion acceleration and other
     * transformations such as normalization.
     *
     * Note that the non-accelerated delta does not represent 'raw' events as
     * they were read from some device. Pointer motion acceleration is device-
     * and configuration-specific and non-accelerated deltas and accelerated
     * deltas may have the same value on some devices.
     *
     * Relative motions are not coupled to Pointer motion events,
     * and can be sent in combination with such events, but also independently. There may
     * also be scenarios where Pointer motion is sent, but there is no
     * relative motion. The order of an absolute and relative motion event
     * originating from the same physical motion is not guaranteed.
     *
     * @param delta Motion vector
     * @param deltaNonAccelerated non-accelerated motion vector
     * @param microseconds timestamp with microseconds granularity
     **/
    void relativeMotion(const QSizeF &delta, const QSizeF &deltaNonAccelerated, quint64 timestamp);

private:
    friend class RelativePointerManager;
    explicit RelativePointer(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
