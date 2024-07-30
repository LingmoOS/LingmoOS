/*
    SPDX-FileCopyrightText: 2017 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_APPMENU_H
#define KWAYLAND_CLIENT_APPMENU_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct org_kde_kwin_appmenu_manager;
struct org_kde_kwin_appmenu;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class AppMenu;

/**
 * @short Wrapper for the org_kde_kwin_appmenu_manager interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_appmenu_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the AppMenuManager interface:
 * @code
 * AppMenuManager *c = registry->createAppMenuManager(name, version);
 * @endcode
 *
 * This creates the AppMenuManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * AppMenuManager *c = new AppMenuManager;
 * c->setup(registry->bindAppMenuManager(name, version));
 * @endcode
 *
 * The AppMenuManager can be used as a drop-in replacement for any org_kde_kwin_appmenu_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.42
 **/
class KWAYLANDCLIENT_EXPORT AppMenuManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new AppMenuManager.
     * Note: after constructing the AppMenuManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use AppMenuManager prefer using
     * Registry::createAppMenuManager.
     **/
    explicit AppMenuManager(QObject *parent = nullptr);
    ~AppMenuManager() override;

    /**
     * Setup this AppMenuManager to manage the @p appmenumanager.
     * When using Registry::createAppMenuManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_appmenu_manager *appmenumanager);
    /**
     * @returns @c true if managing a org_kde_kwin_appmenu_manager.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_appmenu_manager interface.
     * After the interface has been released the AppMenuManager instance is no
     * longer valid and can be setup with another org_kde_kwin_appmenu_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this AppMenuManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_appmenu_manager interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, appmenumanager, &AppMenuManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this AppMenuManager.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this AppMenuManager.
     **/
    EventQueue *eventQueue();

    AppMenu *create(Surface *surface, QObject *parent = nullptr);

    operator org_kde_kwin_appmenu_manager *();
    operator org_kde_kwin_appmenu_manager *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the AppMenuManager got created by
     * Registry::createAppMenuManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 *
 * @since 5.42
 **/
class KWAYLANDCLIENT_EXPORT AppMenu : public QObject
{
    Q_OBJECT
public:
    ~AppMenu() override;

    /**
     * Setup this Appmenu to manage the @p appmenu.
     * When using AppMenuManager::createAppmenu there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_appmenu *appmenu);
    /**
     * @returns @c true if managing a org_kde_kwin_appmenu.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_appmenu interface.
     * After the interface has been released the Appmenu instance is no
     * longer valid and can be setup with another org_kde_kwin_appmenu interface.
     **/
    void release();
    /**
     * Destroys the data held by this Appmenu.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_appmenu interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, appmenu, &Appmenu::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the appmenu address. The DBus object should be registered before making this call
     * Strings should be valid DBus formatted names, in latin1.
     */
    void setAddress(const QString &serviceName, const QString &objectPath);

    operator org_kde_kwin_appmenu *();
    operator org_kde_kwin_appmenu *() const;

private:
    friend class AppMenuManager;
    explicit AppMenu(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
