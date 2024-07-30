/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SUBCOMPOSITOR_H
#define WAYLAND_SUBCOMPOSITOR_H

#include <QObject>
#include <QPointer>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_subcompositor;

namespace KWayland
{
namespace Client
{
class EventQueue;
class SubSurface;
class Surface;

/**
 * @short Wrapper for the wl_subcompositor interface.
 *
 * This class is a convenient wrapper for the wl_subcompositor interface.
 * The main purpose of this class is to create SubSurfaces.
 *
 * To create an instance use Registry::createSubCompositor.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT SubCompositor : public QObject
{
    Q_OBJECT
public:
    explicit SubCompositor(QObject *parent = nullptr);
    ~SubCompositor() override;

    /**
     * @returns @c true if managing a wl_subcompositor.
     **/
    bool isValid() const;
    /**
     * Setup this SubCompositor to manage the @p subcompositor.
     * When using Registry::createSubCompositor there is no need to call this
     * method.
     **/
    void setup(wl_subcompositor *subcompositor);
    /**
     * Releases the wl_subcompositor interface.
     * After the interface has been released the SubCompositor instance is no
     * longer valid and can be setup with another wl_subcompositor interface.
     **/
    void release();
    /**
     * Destroys the data held by this SubCompositor.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_subcompositor interface
     * once there is a new connection available.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a SubSurface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a SubSurface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates and setup a new SubSurface with @p parent.
     * @param parent The parent to pass to the Surface.
     * @returns The new created Surface
     **/
    SubSurface *createSubSurface(QPointer<Surface> surface, QPointer<Surface> parentSurface, QObject *parent = nullptr);

    operator wl_subcompositor *();
    operator wl_subcompositor *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createSubCompositor
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
