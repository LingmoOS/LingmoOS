/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_SERVER_DECORATION_H
#define KWAYLAND_CLIENT_SERVER_DECORATION_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_server_decoration_manager;
struct org_kde_kwin_server_decoration;
struct wl_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class ServerSideDecoration;

/**
 * @short Wrapper for the org_kde_kwin_server_decoration_manager interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_server_decoration_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the ServerSideDecorationManager interface:
 * @code
 * ServerSideDecorationManager *c = registry->createServerSideDecorationManager(name, version);
 * @endcode
 *
 * This creates the ServerSideDecorationManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ServerSideDecorationManager *c = new ServerSideDecorationManager;
 * c->setup(registry->bindServerSideDecorationManager(name, version));
 * @endcode
 *
 * The ServerSideDecorationManager can be used as a drop-in replacement for any org_kde_kwin_server_decoration_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.6
 **/
class KWAYLANDCLIENT_EXPORT ServerSideDecorationManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new ServerSideDecorationManager.
     * Note: after constructing the ServerSideDecorationManager it is not yet valid and one needs
     * to call setup. In order to get a ready-to-use ServerSideDecorationManager prefer using
     * Registry::createServerSideDecorationManager.
     **/
    explicit ServerSideDecorationManager(QObject *parent = nullptr);
    ~ServerSideDecorationManager() override;

    /**
     * Setup this ServerSideDecorationManager to manage the @p serversidedecorationmanager.
     * When using Registry::createServerSideDecorationManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_server_decoration_manager *serversidedecorationmanager);
    /**
     * @returns @c true if managing a org_kde_kwin_server_decoration_manager.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_server_decoration_manager interface.
     * After the interface has been released the ServerSideDecorationManager instance is no
     * longer valid and can be setup with another org_kde_kwin_server_decoration_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this ServerSideDecorationManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_server_decoration_manager interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, serversidedecorationmanager, &ServerSideDecorationManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this ServerSideDecorationManager.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this ServerSideDecorationManager.
     **/
    EventQueue *eventQueue();

    ServerSideDecoration *create(Surface *surface, QObject *parent = nullptr);
    ServerSideDecoration *create(wl_surface *surface, QObject *parent = nullptr);

    operator org_kde_kwin_server_decoration_manager *();
    operator org_kde_kwin_server_decoration_manager *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the ServerSideDecorationManager got created by
     * Registry::createServerSideDecorationManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @brief Describing how a Surface should be decorated.
 *
 * Use ServerSideDecorationManager::create to create a ServerSideDecoration.
 *
 * @see ServerSideDecorationManager
 * @since 5.6
 **/
class KWAYLANDCLIENT_EXPORT ServerSideDecoration : public QObject
{
    Q_OBJECT
public:
    ~ServerSideDecoration() override;

    /**
     * Setup this ServerSideDecoration to manage the @p serversidedecoration.
     * When using ServerSideDecorationManager::createServerSideDecoration there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_server_decoration *serversidedecoration);
    /**
     * @returns @c true if managing a org_kde_kwin_server_decoration.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_server_decoration interface.
     * After the interface has been released the ServerSideDecoration instance is no
     * longer valid and can be setup with another org_kde_kwin_server_decoration interface.
     **/
    void release();
    /**
     * Destroys the data held by this ServerSideDecoration.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_server_decoration interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * ServerDecoration gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Decoration mode used for the Surface.
     **/
    enum class Mode {
        /**
         * Undecorated: neither client, nor server provide decoration. Example: popups.
         **/
        None,
        /**
         * The decoration is part of the surface.
         **/
        Client,
        /**
         * The surface gets embedded into a decoration frame provided by the Server.
         **/
        Server,
    };

    /**
     * Request the decoration @p mode for the Surface.
     *
     * The server will acknowledge the change which will trigger the modeChanged signal.
     *
     * @see mode
     * @see modeChanged
     **/
    void requestMode(Mode mode);

    /**
     * @returns The current decoration mode for the Surface.
     *
     * The mode represents the mode pushed from the Server.
     * @see requestMode
     * @see modeChanged
     **/
    Mode mode() const;

    /**
     * @returns The default decoration mode the server uses
     *
     * @see mode
     **/
    Mode defaultMode() const;

    operator org_kde_kwin_server_decoration *();
    operator org_kde_kwin_server_decoration *() const;

Q_SIGNALS:
    /**
     * Emitted whenever the Server changes the decoration mode for the Surface.
     * @see requestMode
     * @see mode
     **/
    void modeChanged();

private:
    friend class ServerSideDecorationManager;
    explicit ServerSideDecoration(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::ServerSideDecoration::Mode)

#endif
