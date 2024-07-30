/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_BLUR_H
#define KWAYLAND_BLUR_H

#include "buffer.h"

#include <QObject>
#include <QPoint>
#include <QSize>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_buffer;
struct wl_region;
struct org_kde_kwin_blur;
struct org_kde_kwin_blur_manager;


namespace KWayland
{
namespace Client
{
class EventQueue;
class Blur;
class Surface;
class Region;

/**
 * TODO
 */
class KWAYLANDCLIENT_EXPORT BlurManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new BlurManager.
     * Note: after constructing the BlurManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use BlurManager prefer using
     * Registry::createBlurManager.
     **/
    explicit BlurManager(QObject *parent = nullptr);
    ~BlurManager() override;

    /**
     * @returns @c true if managing a org_kde_kwin_blur_manager.
     **/
    bool isValid() const;
    /**
     * Setup this BlurManager to manage the @p compositor.
     * When using Registry::createBlurManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_blur_manager *compositor);
    /**
     * Releases the org_kde_kwin_blur_manager interface.
     * After the interface has been released the BlurManager instance is no
     * longer valid and can be setup with another org_kde_kwin_blur_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this BlurManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_blur_manager interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, compositor, &BlurManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Blur.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Blur.
     **/
    EventQueue *eventQueue();

    /**
     * Creates and setup a new Blur with @p parent.
     * @param parent The parent to pass to the Blur.
     * @returns The new created Blur
     **/
    Blur *createBlur(Surface *surface, QObject *parent = nullptr);
    void removeBlur(Surface *surface);

    operator org_kde_kwin_blur_manager *();
    operator org_kde_kwin_blur_manager *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the BlurManager got created by
     * Registry::createBlurManager
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_kwin_blur interface.
 *
 * This class is a convenient wrapper for the org_kde_kwin_blur interface.
 * To create a Blur call BlurManager::createBlur.
 *
 * The main purpose of this class is to setup the next frame which
 * should be rendered. Therefore it provides methods to add damage
 * and to attach a new Buffer and to finalize the frame by calling
 * commit.
 *
 * @see BlurManager
 **/
class KWAYLANDCLIENT_EXPORT Blur : public QObject
{
    Q_OBJECT
public:
    ~Blur() override;

    /**
     * Setup this Blur to manage the @p blur.
     * When using BlurManager::createBlur there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_blur *blur);
    /**
     * Releases the org_kde_kwin_blur interface.
     * After the interface has been released the Blur instance is no
     * longer valid and can be setup with another org_kde_kwin_blur interface.
     **/
    void release();
    /**
     * Destroys the data held by this Blur.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_blur interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * Blur gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a org_kde_kwin_blur.
     **/
    bool isValid() const;

    void commit();

    /**
     * Sets the area of the window that will have a blurred
     * background.
     * The region will have to be created with
     * Compositor::createRegion(QRegion)
     */
    void setRegion(Region *region);

    operator org_kde_kwin_blur *();
    operator org_kde_kwin_blur *() const;

private:
    friend class BlurManager;
    explicit Blur(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
