/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef LINGMO_QUICK_REGISTRY_H
#define LINGMO_QUICK_REGISTRY_H

#include <QHash>
#include <QObject>

#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/connection_thread.h>

struct wl_registry;

struct lingmo_shell;
struct lingmo_window_management;
struct org_kde_plasma_virtual_desktop_management;

namespace KWayland::Client
{
class PlasmaVirtualDesktopManagement;
}
class LingmoUIShell;
class LingmoUIWindowManagement;

namespace LingmoUIQuick::WaylandClient
{
/**
 * @short Wrapper for the wl_registry interface.
 *
 * The purpose of this class is to manage the wl_registry interface.
 * This class supports some well-known interfaces and can create a
 * wrapper class for those.
 *
 * The main purpose is to emit signals whenever a new interface is
 * added or an existing interface is removed. For the well known interfaces
 * dedicated signals are emitted allowing a user to connect directly to the
 * signal announcing the interface it is interested in.
 *
 * To create and setup the Registry one needs to call create with either a
 * wl_display from an existing Wayland connection or a ConnectionThread instance:
 *
 * @code
 * ConnectionThread *connection; // existing connection
 * Registry registry;
 * registry.create(connection);
 * registry.setup();
 * @endcode
 *
 * The interfaces are announced in an asynchronous way by the Wayland server.
 * To initiate the announcing of the interfaces one needs to call setup.
 **/
class Registry : public QObject
{
Q_OBJECT
public:
    /**
     * The well-known interfaces this Registry supports.
     * For each of the enum values the Registry is able to create a Wrapper
     * object.
     **/
    enum class Interface {
        Unknown, ///< Refers to an Unknown interface
        LingmoUIShell,
        LingmoUIWindowManagement,
        PlasmaVirtualDesktopManagement, ///< Refers to org_kde_plasma_virtual_desktop_management interface
    };
    explicit Registry(QObject *parent = nullptr);
    ~Registry() override;

    /**
     * Releases the wl_registry interface.
     * After the interface has been released the Registry instance is no
     * longer valid and can be setup with another wl_registry interface.
     **/
    void release();
    /**
     * Destroys the data held by this Registry.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_registry interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, registry, &Registry::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Gets the registry from the @p display.
     **/
    void create(wl_display *display);
    /**
     * Gets the registry from the @p connection.
     **/
    void create(KWayland::Client::ConnectionThread *connection);
    /**
     * Finalizes the setup of the Registry.
     * After calling this method the interfaces will be announced in an asynchronous way.
     * The Registry must have been created when calling this method.
     * @see create
     **/
    void setup();

    /**
     * Sets the @p queue to use for this Registry.
     *
     * The EventQueue should be set before the Registry gets setup.
     * The EventQueue gets automatically added to all interfaces created by
     * this Registry. So that all objects are in the same EventQueue.
     *
     * @param queue The event queue to use for this Registry.
     **/
    void setEventQueue(KWayland::Client::EventQueue *queue);
    /**
     * @returns The EventQueue used by this Registry
     **/
    KWayland::Client::EventQueue *eventQueue();

    /**
     * @returns @c true if managing a wl_registry.
     **/
    bool isValid() const;
    /**
     * @returns @c true if the Registry has an @p interface.
     **/
    bool hasInterface(Interface interface) const;

    /**
     * Representation of one announced interface.
     **/
    struct AnnouncedInterface {
        /**
         * The name of the announced interface.
         **/
        quint32 name;
        /**
         * The maximum supported version of the announced interface.
         **/
        quint32 version;
    };
    /**
     * Provides name and version for the @p interface.
     *
     * The first value of the returned pair is the "name", the second value is the "version".
     * If the @p interface has not been announced, both values are set to 0.
     * If there @p interface has been announced multiple times, the last announced is returned.
     * In case one is interested in all announced interfaces, one should prefer @link interfaces(Interface) @endlink.
     *
     * The returned information can be passed into the bind or create methods.
     *
     * @param interface The well-known interface for which the name and version should be retrieved
     * @returns name and version of the given interface
     * @since 5.5
     **/
    AnnouncedInterface interface(Interface interface) const;
    /**
     * Provides all pairs of name and version for the well-known @p interface.
     *
     * If the @p interface has not been announced, an empty vector is returned.
     *
     * The returned information can be passed into the bind or create methods.
     *
     * @param interface The well-known interface for which the name and version should be retrieved
     * @returns All pairs of name and version of the given interface
     * @since 5.5
     **/
    QVector<AnnouncedInterface> interfaces(Interface interface) const;

    lingmo_shell *bindLingmoUIShell(uint32_t name, uint32_t version) const;
    lingmo_window_management *bindLingmoUIWindowManagement(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_virtual_desktop_management with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma Virtual desktop interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaShell instead.
     * @see createPlasmaShell
     **/
    org_kde_plasma_virtual_desktop_management *bindPlasmaVirtualDesktopManagement(uint32_t name, uint32_t version) const;

    LingmoUIShell *createLingmoUIShell(quint32 name, quint32 version, QObject *parent = nullptr);
    LingmoUIWindowManagement *createLingmoUIWindowManagement(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaVirtualDesktopManagement and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_virtual_desktop_management interface,
     * the returned VirtualDesktop will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_virtual_desktop_management interface to bind
     * @param version The version or the org_kde_plasma_virtual_desktop_management interface to use
     * @param parent The parent for PlasmaShell
     *
     * @returns The created PlasmaShell.
     **/
    KWayland::Client::PlasmaVirtualDesktopManagement *createPlasmaVirtualDesktopManagement(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * cast operator to the low-level Wayland @c wl_registry
     **/
    operator wl_registry *();
    /**
     * cast operator to the low-level Wayland @c wl_registry
     **/
    operator wl_registry *() const;
    /**
     * @returns access to the low-level Wayland @c wl_registry
     **/
    wl_registry *registry();

Q_SIGNALS:
    /**
     * @name Interface announced signals.
     **/
    /**
    * Emitted whenever a lingmo_window_management_interface interface gets announced.
    * @param name The name for the announced interface
    * @param version The maximum supported version of the announced interface
    **/
    void lingmoWindowManagementAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a lingmo_shell_interface interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void lingmoShellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_plasma_virtual_desktop_management interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void plasmaVirtualDesktopManagementAnnounced(quint32 name, quint32 version);

    ///@}

    /**
     * @name Interface removed signals.
     **/
    /**
     ** Generic announced signal which gets emitted whenever an interface gets
     * announced.
     *
     * This signal is emitted before the dedicated signals are handled. If one
     * wants to know about one of the well-known interfaces use the dedicated
     * signals instead. Especially the bind methods might fail before the dedicated
     * signals are emitted.
     *
     * @param interface The interface (e.g. wl_compositor) which is announced
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void interfaceAnnounced(QByteArray interface, quint32 name, quint32 version);
    /**
     * Emitted when the Wayland display is done flushing the initial interface
     * callbacks, announcing wl_display properties. This can be used to compress
     * events. Note that this signal is emitted only after announcing interfaces,
     * such as outputs, but not after receiving callbacks of interface properties,
     * such as the output's geometry, modes, etc..
     * This signal is emitted from the wl_display_sync callback.
     **/
    void interfacesAnnounced();
    /**
     * Generic removal signal which gets emitted whenever an interface gets removed.
     *
     * This signal is emitted after the dedicated signals are handled.
     *
     * @param name The name for the removed interface
     **/
    void interfaceRemoved(quint32 name);
    /**
     * Emitted whenever a lingmo_shell interface gets removed.
     * @param name The name for the removed interface
     **/
    void lingmoShellRemoved(quint32 name);
    /**
     * Emitted whenever a lingmo_window_management interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void lingmoWindowManagementRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_virtual_desktop_management interface gets removed.
     * @param name The name for the removed interface
     * @since 5.52
     **/
    void plasmaVirtualDesktopManagementRemoved(quint32 name);

Q_SIGNALS:
    /*
     * Emitted when the registry has been destroyed rather than released
     */
    void registryDestroyed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}


#endif //LINGMO_QUICK_REGISTRY_H
