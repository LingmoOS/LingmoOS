/*
    SPDX-FileCopyrightText: 2017 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_SERVER_DECORATION_PALETTE_H
#define KWAYLAND_CLIENT_SERVER_DECORATION_PALETTE_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_server_decoration_palette_manager;
struct org_kde_kwin_server_decoration_palette;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class ServerSideDecorationPalette;

/**
 * @short Wrapper for the org_kde_kwin_server_decoration_palette_manager interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_server_decoration_palette_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the ServerSideDecorationPaletteManager interface:
 * @code
 * ServerSideDecorationPaletteManager *c = registry->createServerSideDecorationPaletteManager(name, version);
 * @endcode
 *
 * This creates the ServerSideDecorationPaletteManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ServerSideDecorationPaletteManager *c = new ServerSideDecorationPaletteManager;
 * c->setup(registry->bindServerSideDecorationPaletteManager(name, version));
 * @endcode
 *
 * The ServerSideDecorationPaletteManager can be used as a drop-in replacement for any org_kde_kwin_server_decoration_palette_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT ServerSideDecorationPaletteManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new ServerSideDecorationPaletteManager.
     * Note: after constructing the ServerSideDecorationPaletteManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use ServerSideDecorationPaletteManager prefer using
     * Registry::createServerSideDecorationPaletteManager.
     **/
    explicit ServerSideDecorationPaletteManager(QObject *parent = nullptr);
    ~ServerSideDecorationPaletteManager() override;

    /**
     * Setup this ServerSideDecorationPaletteManager to manage the @p serverSideDecorationPaletteManager.
     * When using Registry::createServerSideDecorationPaletteManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_server_decoration_palette_manager *serverSideDecorationPaletteManager);
    /**
     * @returns @c true if managing a org_kde_kwin_server_decoration_palette_manager.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_server_decoration_palette_manager interface.
     * After the interface has been released the ServerSideDecorationPaletteManager instance is no
     * longer valid and can be setup with another org_kde_kwin_server_decoration_palette_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this ServerSideDecorationPaletteManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_server_decoration_palette_manager interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, palettemanager, &ServerSideDecorationPaletteManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this ServerSideDecorationPaletteManager.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this ServerSideDecorationPaletteManager.
     **/
    EventQueue *eventQueue();

    ServerSideDecorationPalette *create(Surface *surface, QObject *parent = nullptr);

    operator org_kde_kwin_server_decoration_palette_manager *();
    operator org_kde_kwin_server_decoration_palette_manager *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the ServerSideDecorationPaletteManager got created by
     * Registry::createServerSideDecorationPaletteManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT ServerSideDecorationPalette : public QObject
{
    Q_OBJECT
public:
    ~ServerSideDecorationPalette() override;

    /**
     * Setup this ServerSideDecorationPalette to manage the @p serversidedecorationpalette.
     * When using ServerSideDecorationPaletteManager::create there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_server_decoration_palette *serversidedecorationpalette);
    /**
     * @returns @c true if managing a org_kde_kwin_server_decoration_palette.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_server_decoration_palette interface.
     * After the interface has been released the ServerSideDecorationPalette instance is no
     * longer valid and can be setup with another org_kde_kwin_server_decoration_palette interface.
     **/
    void release();
    /**
     * Destroys the data held by this ServerSideDecorationPalette.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_server_decoration_palette interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, palette, &ServerSideDecorationPalette::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the palette to be used by the server side decorations.
     * Absolute file path, or name of palette in the user's config directory.
     * If set to empty the default palette will be used.
     */
    void setPalette(const QString &palette);

    operator org_kde_kwin_server_decoration_palette *();
    operator org_kde_kwin_server_decoration_palette *() const;

private:
    friend class ServerSideDecorationPaletteManager;
    explicit ServerSideDecorationPalette(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
