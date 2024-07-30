/*
    SPDX-FileCopyrightText: 2017 Martin Flöser <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_IDLEINHIBIT_H
#define KWAYLAND_CLIENT_IDLEINHIBIT_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct zwp_idle_inhibit_manager_v1;
struct zwp_idle_inhibitor_v1;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class IdleInhibitor;

/**
 * @short Wrapper for the zwp_idle_inhibit_manager_v1 interface.
 *
 * This class provides a convenient wrapper for the zwp_idle_inhibit_manager_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the IdleInhibitManager interface:
 * @code
 * IdleInhibitManager *c = registry->createIdleInhibitManager(name, version);
 * @endcode
 *
 * This creates the IdleInhibitManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * IdleInhibitManager *c = new IdleInhibitManager;
 * c->setup(registry->bindIdleInhibitManager(name, version));
 * @endcode
 *
 * The IdleInhibitManager can be used as a drop-in replacement for any zwp_idle_inhibit_manager_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.41
 **/
class KWAYLANDCLIENT_EXPORT IdleInhibitManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new IdleInhibitManager.
     * Note: after constructing the IdleInhibitManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use IdleInhibitManager prefer using
     * Registry::createIdleInhibitManager.
     **/
    explicit IdleInhibitManager(QObject *parent = nullptr);
    ~IdleInhibitManager() override;

    /**
     * Setup this IdleInhibitManager to manage the @p idleinhibitmanager.
     * When using Registry::createIdleInhibitManager there is no need to call this
     * method.
     **/
    void setup(zwp_idle_inhibit_manager_v1 *idleinhibitmanager);
    /**
     * @returns @c true if managing a zwp_idle_inhibit_manager_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_idle_inhibit_manager_v1 interface.
     * After the interface has been released the IdleInhibitManager instance is no
     * longer valid and can be setup with another zwp_idle_inhibit_manager_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this IdleInhibitManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_idle_inhibit_manager_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, idleinhibitmanager, &IdleInhibitManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this IdleInhibitManager.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this IdleInhibitManager.
     **/
    EventQueue *eventQueue();

    /**
     * Creates an IdleInhibitor for the given @p surface.
     * While the IdleInhibitor exists the @p surface is marked to inhibit idle.
     * @param surface The Surface which should have idle inhibited
     * @param parent The parent object for the IdleInhibitor
     * @returns The created IdleInhibitor
     **/
    IdleInhibitor *createInhibitor(Surface *surface, QObject *parent = nullptr);

    operator zwp_idle_inhibit_manager_v1 *();
    operator zwp_idle_inhibit_manager_v1 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the IdleInhibitManager got created by
     * Registry::createIdleInhibitManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * An IdleInhibitor prevents the Output that the associated Surface is visible on from being
 * set to a state where it is not visually usable due to lack of user interaction
 * (e.g. blanked, dimmed, locked, set to power save, etc.)  Any screensaver processes are
 * also blocked from displaying.
 *
 * If the Surface is destroyed, unmapped, becomes occluded, loses visibility, or otherwise
 * becomes not visually relevant for the user, the IdleInhibitor will not be honored by
 * the compositor; if the Surface subsequently regains visibility the inhibitor takes effect
 * once again.
 * Likewise, the IdleInhibitor isn't honored if the system was already idled at the time the
 * IdleInhibitor was established, although if the system later de-idles and re-idles the
 * IdleInhibitor will take effect.
 *
 * @see IdleInhibitManager
 * @see Surface
 * @since 5.41
 **/
class KWAYLANDCLIENT_EXPORT IdleInhibitor : public QObject
{
    Q_OBJECT
public:
    ~IdleInhibitor() override;

    /**
     * Setup this IdleInhibitor to manage the @p idleinhibitor.
     * When using IdleInhibitManager::createIdleInhibitor there is no need to call this
     * method.
     **/
    void setup(zwp_idle_inhibitor_v1 *idleinhibitor);
    /**
     * @returns @c true if managing a zwp_idle_inhibitor_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zwp_idle_inhibitor_v1 interface.
     * After the interface has been released the IdleInhibitor instance is no
     * longer valid and can be setup with another zwp_idle_inhibitor_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this IdleInhibitor.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_idle_inhibitor_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, idleinhibitor, &IdleInhibitor::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    operator zwp_idle_inhibitor_v1 *();
    operator zwp_idle_inhibitor_v1 *() const;

private:
    friend class IdleInhibitManager;
    explicit IdleInhibitor(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
