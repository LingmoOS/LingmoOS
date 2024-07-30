/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_SHADOW_H
#define KWAYLAND_SHADOW_H

#include "buffer.h"

#include <QObject>
#include <QPoint>
#include <QSize>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_buffer;
struct org_kde_kwin_shadow;
struct org_kde_kwin_shadow_manager;

class QMarginsF;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Shadow;
class Surface;

/**
 * @short Wrapper for the org_kde_kwin_shadow_manager interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_shadow_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the ShadowManager interface:
 * @code
 * ShadowManager *s = registry->createShadowManager(name, version);
 * @endcode
 *
 * This creates the ShadowManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ShadowManager *s = new ShadowManager;
 * s->setup(registry->bindShadowManager(name, version));
 * @endcode
 *
 * The ShadowManager can be used as a drop-in replacement for any org_kde_kwin_shadow_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.4
 **/
class KWAYLANDCLIENT_EXPORT ShadowManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new ShadowManager.
     * Note: after constructing the ShadowManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use ShadowManager prefer using
     * Registry::createShadowManager.
     **/
    explicit ShadowManager(QObject *parent = nullptr);
    ~ShadowManager() override;

    /**
     * @returns @c true if managing a org_kde_kwin_shadow_manager.
     **/
    bool isValid() const;
    /**
     * Setup this ShadowManager to manage the @p compositor.
     * When using Registry::createShadowManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_shadow_manager *compositor);
    /**
     * Releases the org_kde_kwin_shadow_manager interface.
     * After the interface has been released the ShadowManager instance is no
     * longer valid and can be setup with another org_kde_kwin_shadow_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this ShadowManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_shadow_manager interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * Shadow gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Shadow.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Shadow.
     **/
    EventQueue *eventQueue();

    /**
     * Creates and setup a new Shadow with @p parent.
     * @param parent The parent to pass to the Shadow.
     * @returns The new created Shadow
     **/
    Shadow *createShadow(Surface *surface, QObject *parent = nullptr);
    void removeShadow(Surface *surface);

    operator org_kde_kwin_shadow_manager *();
    operator org_kde_kwin_shadow_manager *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createShadowManager
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_kwin_shadow interface.
 *
 * This class is a convenient wrapper for the org_kde_kwin_shadow interface.
 * To create a Shadow call Compositor::createShadow.
 *
 * The main purpose of this class is to setup the next frame which
 * should be rendered. Therefore it provides methods to add damage
 * and to attach a new Buffer and to finalize the frame by calling
 * commit.
 *
 * @see Compositor
 **/
class KWAYLANDCLIENT_EXPORT Shadow : public QObject
{
    Q_OBJECT
public:
    ~Shadow() override;

    /**
     * Setup this Shadow to manage the @p shadow.
     * When using Compositor::createSurface there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_shadow *shadow);
    /**
     * Releases the org_kde_kwin_shadow interface.
     * After the interface has been released the Shadow instance is no
     * longer valid and can be setup with another org_kde_kwin_shadow interface.
     **/
    void release();
    /**
     * Destroys the data held by this Shadow.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_shadow interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, shadow, &Shadow::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a org_kde_kwin_shadow.
     **/
    bool isValid() const;

    void commit();
    void attachLeft(wl_buffer *buffer);
    void attachLeft(Buffer *buffer);
    void attachLeft(Buffer::Ptr buffer);
    void attachTopLeft(wl_buffer *buffer);
    void attachTopLeft(Buffer *buffer);
    void attachTopLeft(Buffer::Ptr buffer);
    void attachTop(wl_buffer *buffer);
    void attachTop(Buffer *buffer);
    void attachTop(Buffer::Ptr buffer);
    void attachTopRight(wl_buffer *buffer);
    void attachTopRight(Buffer *buffer);
    void attachTopRight(Buffer::Ptr buffer);
    void attachRight(wl_buffer *buffer);
    void attachRight(Buffer *buffer);
    void attachRight(Buffer::Ptr buffer);
    void attachBottomRight(wl_buffer *buffer);
    void attachBottomRight(Buffer *buffer);
    void attachBottomRight(Buffer::Ptr buffer);
    void attachBottom(wl_buffer *buffer);
    void attachBottom(Buffer *buffer);
    void attachBottom(Buffer::Ptr buffer);
    void attachBottomLeft(wl_buffer *buffer);
    void attachBottomLeft(Buffer *buffer);
    void attachBottomLeft(Buffer::Ptr buffer);
    void setOffsets(const QMarginsF &margins);

    operator org_kde_kwin_shadow *();
    operator org_kde_kwin_shadow *() const;

private:
    friend class ShadowManager;
    explicit Shadow(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
