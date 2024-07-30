/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_POINTERCONSTRAINTS_H
#define KWAYLAND_CLIENT_POINTERCONSTRAINTS_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct zwp_pointer_constraints_v1;
struct zwp_locked_pointer_v1;
struct zwp_confined_pointer_v1;

class QPointF;

namespace KWayland
{
namespace Client
{
class EventQueue;
class LockedPointer;
class Surface;
class Region;
class ConfinedPointer;
class Pointer;

/**
 * @short Wrapper for the zwp_pointer_constraints_v1 interface.
 *
 * This class provides a convenient wrapper for the zwp_pointer_constraints_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the PointerConstraints interface:
 * @code
 * PointerConstraints *c = registry->createPointerConstraints(name, version);
 * @endcode
 *
 * This creates the PointerConstraints and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * PointerConstraints *c = new PointerConstraints;
 * c->setup(registry->bindPointerConstraints(name, version));
 * @endcode
 *
 * The PointerConstraints can be used as a drop-in replacement for any zwp_pointer_constraints_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.29
 **/
class KWAYLANDCLIENT_EXPORT PointerConstraints : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new PointerConstraints.
     * Note: after constructing the PointerConstraints it is not yet valid and one needs
     * to call setup. In order to get a ready to use PointerConstraints prefer using
     * Registry::createPointerConstraints.
     **/
    explicit PointerConstraints(QObject *parent = nullptr);
    ~PointerConstraints() override;

    /**
     * Setup this PointerConstraints to manage the @p pointerconstraints.
     * When using Registry::createPointerConstraints there is no need to call this
     * method.
     **/
    void setup(zwp_pointer_constraints_v1 *pointerconstraints);
    /**
     * @returns @c true if managing a zwp_pointer_constraints_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_pointer_constraints_v1 interface.
     * After the interface has been released the PointerConstraints instance is no
     * longer valid and can be setup with another zwp_pointer_constraints_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this PointerConstraints.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_pointer_constraints_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, pointerconstraints, &PointerConstraints::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this PointerConstraints.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this PointerConstraints.
     **/
    EventQueue *eventQueue();

    /**
     * These values represent different lifetime semantics. They are passed
     * as arguments to the factory requests to specify how the constraint
     * lifetimes should be managed.
     * @see lockPointer
     * @see confinePointer
     **/
    enum class LifeTime {
        /**
         * A OneShot pointer constraint will never reactivate once it has been
         * deactivated.
         **/
        OneShot,
        /**
         * A persistent pointer constraint may again reactivate once it has
         * been deactivated.
         **/
        Persistent,
    };

    /**
     * This factory method creates a LockedPointer.
     *
     * A LockedPointer lets the client request to disable movements of
     * the virtual pointer (i.e. the cursor), effectively locking the pointer
     * to a position.
     *
     * Creating a LockedPointer does not lock the pointer immediately; in the
     * future, when the compositor deems implementation-specific constraints
     * are satisfied, the pointer lock will be activated and the compositor
     * sends a locked event, reported by {@link LockedPointer::locked}.
     *
     * The protocol provides no guarantee that the constraints are ever
     * satisfied, and does not require the compositor to send an error if the
     * constraints cannot ever be satisfied. It is thus possible to request a
     * lock that will never activate.
     *
     * There may not be another pointer constraint of any kind requested or
     * active on the @p surface for any of the Pointer objects of the Seat of
     * the passed @p pointer when requesting a lock. If there is, an error will be
     * raised.
     *
     * The intersection of the @p region passed with this request and the input
     * region of the @p surface is used to determine where the pointer must be
     * in order for the lock to activate. It is up to the compositor whether to
     * warp the pointer or require some kind of user interaction for the lock
     * to activate. If the @p region is null the surface input region is used.
     *
     * A Surface may receive pointer focus without the lock being activated.
     *
     * Note that while a pointer is locked, the Pointer objects of the
     * corresponding seat will not emit any {@link Pointer::motion} signals, but
     * relative motion events will still be emitted via {@link RelativePointer::relativeMotion}.
     * Pointer axis and button events are unaffected.
     *
     * @param surface The Surface which should be constrained in pointer motion
     * @param pointer The Pointer object for which this LockedPointer should be created
     * @param region Region where to lock the pointer, if @c null the input region of the Surface is used
     * @param lifetime Whether the LockedPointer becomes invalid on unlocked
     * @param parent The parent object for the LockedPointer
     * @returns The factored LockedPointer
     **/
    LockedPointer *lockPointer(Surface *surface, Pointer *pointer, Region *region, LifeTime lifetime, QObject *parent = nullptr);

    /**
     * This factory method creates a ConfinedPointer.
     *
     * A ConfinedPointer lets the client request to confine the
     * pointer cursor to a given @p region. Creating a ConfinedPointer
     * does not take effect immediately; in the future, when the compositor
     * deems implementation-specific constraints are satisfied, the pointer
     * confinement will be	activated and the compositor sends a confined event,
     * which is reported through the {@link ConfinedPointer::confined} signal.
     *
     * The intersection of the @p region passed and the input region of the
     * @p surface is used to determine where the pointer must be
     * in order for the confinement to activate. It is up to the compositor
     * whether to warp the pointer or require some kind of user interaction for
     * the confinement to activate. If the @p region is @c null the @p surface input
     * region is used.
     *
     * @param surface The Surface which should be constrained in pointer motion
     * @param pointer The Pointer object for which this LockedPointer should be created
     * @param region Region where to confine the pointer, if @c null the input region of the Surface is used
     * @param lifetime Whether the ConfinedPointer becomes invalid on unconfined
     * @param parent The parent object for the ConfinedPointer
     * @returns The factored ConfinedPointer
     **/
    ConfinedPointer *confinePointer(Surface *surface, Pointer *pointer, Region *region, LifeTime lifetime, QObject *parent = nullptr);

    operator zwp_pointer_constraints_v1 *();
    operator zwp_pointer_constraints_v1 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the PointerConstraints got created by
     * Registry::createPointerConstraints
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the zwp_locked_pointer_v1 interface.
 *
 * The LockedPointer represents a locked pointer state.
 *
 * While the lock of this object is active, the Pointer objects of the
 * associated seat will not emit any {@link Pointer::motion} events.
 *
 * This object will send the signal locked when the lock is activated.
 * Whenever the lock is activated, it is guaranteed that the locked surface
 * will already have received pointer focus and that the pointer will be
 * within the region passed to the request creating this object.
 *
 * To unlock the pointer, delete the object.
 *
 * If the compositor decides to unlock the pointer the unlocked signal is
 * emitted.
 *
 * When unlocking, the compositor may warp the cursor position to the set
 * cursor position hint. If it does, it will not result in any relative
 * motion events emitted via {@link RelativePointer::relativeMotion}.
 *
 * If the Surface the lock was requested on is destroyed and the lock is not
 * yet activated, the LockedPointer object is now defunct and must be
 * deleted.
 *
 * @see PointerConstraints::lockedPointer
 * @since 5.29
 **/
class KWAYLANDCLIENT_EXPORT LockedPointer : public QObject
{
    Q_OBJECT
public:
    ~LockedPointer() override;

    /**
     * Setup this LockedPointer to manage the @p lockedpointer.
     * When using PointerConstraints::createLockedPointer there is no need to call this
     * method.
     **/
    void setup(zwp_locked_pointer_v1 *lockedpointer);
    /**
     * @returns @c true if managing a zwp_locked_pointer_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_locked_pointer_v1 interface.
     * After the interface has been released the LockedPointer instance is no
     * longer valid and can be setup with another zwp_locked_pointer_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this LockedPointer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_locked_pointer_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, lockedpointer, &LockedPointer::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Set the cursor position hint relative to the top left corner of the Surface.
     *
     * If the client is drawing its own cursor, it should update the position
     * hint to the position of its own cursor. A compositor may use this
     * information to warp the pointer upon unlock in order to avoid pointer
     * jumps.
     *
     * The cursor position hint is double buffered. The new hint will only take
     * effect when the associated surface gets it pending state applied.
     * See {@link Surface::commit} for details.
     *
     * @param surfaceLocal The new position hint in surface local coordinates
     * @see Surface::commit
     **/
    void setCursorPositionHint(const QPointF &surfaceLocal);

    /**
     * Set a new region used to lock the pointer.
     *
     * The new lock region is double-buffered. The new lock region will
     * only take effect when the associated Surface gets its pending state
     * applied. See {@link Surface::commit} for details.
     *
     * @param region The new lock region.
     * @see Surface::commit
     * @see PointerConstraints::lockPointer
     **/
    void setRegion(Region *region);

    operator zwp_locked_pointer_v1 *();
    operator zwp_locked_pointer_v1 *() const;

Q_SIGNALS:
    /**
     * Notification that the pointer lock of the seat's pointer is activated.
     * @see unlocked
     **/
    void locked();

    /**
     * Notification that the pointer lock of the seat's pointer is no longer
     * active. If this is a oneshot pointer lock (see
     * wp_pointer_constraints.lifetime) this object is now defunct and should
     * be destroyed. If this is a persistent pointer lock (see
     * wp_pointer_constraints.lifetime) this pointer lock may again
     * reactivate in the future.
     * @see locked
     **/
    void unlocked();

private:
    friend class PointerConstraints;
    explicit LockedPointer(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for zwp_confined_pointer_v1 protocol
 * The confine pointer interface represents a confined pointer state.
 *
 * This object will send the signal 'confined' when the confinement is
 * activated. Whenever the confinement is activated, it is guaranteed that
 * the surface the pointer is confined to will already have received pointer
 * focus and that the pointer will be within the region passed to the request
 * creating this object. It is up to the compositor to decide whether this
 * requires some user interaction and if the pointer will warp to within the
 * passed region if outside.
 *
 * To unconfine the pointer, delete the object.
 *
 * If the compositor decides to unconfine the pointer the unconfined signal is
 * emitted. The ConfinedPointer object is at this point defunct and should
 * be deleted.
 * @see PointerConstraints::confinePointer
 * @since 5.29
 **/
class KWAYLANDCLIENT_EXPORT ConfinedPointer : public QObject
{
    Q_OBJECT
public:
    ~ConfinedPointer() override;

    /**
     * Setup this ConfinedPointer to manage the @p confinedpointer.
     * When using PointerConstraints::createConfinedPointer there is no need to call this
     * method.
     **/
    void setup(zwp_confined_pointer_v1 *confinedpointer);
    /**
     * @returns @c true if managing a zwp_confined_pointer_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_confined_pointer_v1 interface.
     * After the interface has been released the ConfinedPointer instance is no
     * longer valid and can be setup with another zwp_confined_pointer_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this ConfinedPointer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_confined_pointer_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * PointerConstraints gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Set a new region used to confine the pointer.
     *
     * The new confine region is double-buffered. The new confine region will
     * only take effect when the associated Surface gets its pending state
     * applied. See {@link Surface::commit} for details.
     *
     * If the confinement is active when the new confinement region is applied
     * and the pointer ends up outside of newly applied region, the pointer may
     * warped to a position within the new confinement region. If warped, a
     * {@link Pointer::motion} signal will be emitted, but no
     * {@link RelativePointer::relativeMotion} signal.
     *
     * The compositor may also, instead of using the new region, unconfine the
     * pointer.
     *
     * @param region The new confine region.
     * @see Surface::commit
     * @see PointerConstraints::confinePointer
     **/
    void setRegion(Region *region);

    operator zwp_confined_pointer_v1 *();
    operator zwp_confined_pointer_v1 *() const;

Q_SIGNALS:
    /**
     * Notification that the pointer confinement of the seat's pointer is activated.
     * @see unconfined
     **/
    void confined();

    /**
     * Notification that the pointer confinement of the seat's pointer is no
     * longer active. If this is a oneshot pointer confinement (see
     * wp_pointer_constraints.lifetime) this object is now defunct and should
     * be destroyed. If this is a persistent pointer confinement (see
     * wp_pointer_constraints.lifetime) this pointer confinement may again
     * reactivate in the future.
     * @see confined
     **/
    void unconfined();

private:
    friend class PointerConstraints;
    explicit ConfinedPointer(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
