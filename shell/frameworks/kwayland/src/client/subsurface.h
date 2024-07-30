/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SUBSURFACE_H
#define WAYLAND_SUBSURFACE_H

#include <QObject>
#include <QPointer>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_subsurface;

namespace KWayland
{
namespace Client
{
class Surface;

/**
 * @short Wrapper for the wl_subsurface interface.
 *
 * This class is a convenient wrapper for the wl_subsurface interface.
 * To create a SubSurface call SubCompositor::createSubSurface.
 *
 * A SubSurface is bound to a Surface and has a parent Surface.
 * A SubSurface can only be created for a Surface not already used in onther way,
 * e.g. as a ShellSurface.
 *
 * The SubSurface has a position in local coordinates to the parent Surface.
 * Please note that changing the position is a double buffered state and is only
 * applied once the parent surface is committed. The same applies for manipulating
 * the stacking order of the SubSurface's siblings.
 *
 * @see SubCompositor
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT SubSurface : public QObject
{
    Q_OBJECT
public:
    explicit SubSurface(QPointer<Surface> surface, QPointer<Surface> parentSurface, QObject *parent = nullptr);
    ~SubSurface() override;

    /**
     * @returns @c true if managing a wl_subsurface.
     **/
    bool isValid() const;
    /**
     * Setup this SubSurface to manage the @p subsurface.
     * When using SubCompositor::createSubSurface there is no need to call this
     * method.
     **/
    void setup(wl_subsurface *subsurface);
    /**
     * Releases the wl_subsurface interface.
     * After the interface has been released the SubSurface instance is no
     * longer valid and can be setup with another wl_subsurface interface.
     **/
    void release();
    /**
     * Destroys the data held by this SubSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_subsurface interface
     * once there is a new connection available.
     *
     * @see release
     **/
    void destroy();

    /**
     * Operation Mode on how the Surface's commit should behave.
     **/
    enum class Mode {
        Synchronized,
        Desynchronized,
    };

    /**
     * Sets the operation mode to @p mode.
     * Initially a SubSurface is in Synchronized Mode.
     **/
    void setMode(Mode mode);
    Mode mode() const;

    /**
     * Sets the position in relative coordinates to the parent surface to @p pos.
     *
     * The change is only applied after the parent Surface got committed.
     **/
    void setPosition(const QPoint &pos);
    QPoint position() const;

    /**
     * Raises this SubSurface above all siblings.
     * This is the same as calling placeAbove with the parent surface
     * as argument.
     *
     * The change is only applied after the parent surface got committed.
     * @see placeAbove
     **/
    void raise();
    /**
     * Places the SubSurface above the @p sibling.
     *
     * The change is only applied after the parent surface got committed.
     * @param sibling The SubSurface on top of which this SubSurface should be placed
     **/
    void placeAbove(QPointer<SubSurface> sibling);
    /**
     * Places the SubSurface above the @p referenceSurface.
     *
     * In case @p referenceSurface is the parent surface this SubSurface is
     * raised to the top of the stacking order. Otherwise it is put directly
     * above the @p referenceSurface in the stacking order.
     *
     * The change is only applied after the parent surface got committed.
     * @param referenceSurface Either a sibling or parent Surface
     **/
    void placeAbove(QPointer<Surface> referenceSurface);

    /**
     * Lowers this SubSurface below all siblings.
     * This is the same as calling placeBelow with the parent surface
     * as argument.
     *
     * The change is only applied after the parent surface got committed.
     * @see placeBelow
     **/
    void lower();
    /**
     * Places the SubSurface below the @p sibling.
     *
     * The change is only applied after the parent surface got committed.
     * @param sibling The SubSurface under which the SubSurface should be put
     **/
    void placeBelow(QPointer<SubSurface> sibling);
    /**
     * Places the SubSurface below the @p referenceSurface.
     *
     * In case @p referenceSurface is the parent surface this SubSurface is
     * lowered to the bottom of the stacking order. Otherwise it is put directly
     * below the @p referenceSurface in the stacking order.
     *
     * The change is only applied after the parent surface got committed.
     * @param referenceSurface Either a sibling or parent Surface
     **/
    void placeBelow(QPointer<Surface> referenceSurface);

    /**
     * @returns The Surface for which this SubSurface got created.
     **/
    QPointer<Surface> surface() const;
    /**
     * @returns The parent Surface of this SubSurface.
     **/
    QPointer<Surface> parentSurface() const;

    static QPointer<SubSurface> get(wl_subsurface *native);

    operator wl_subsurface *();
    operator wl_subsurface *() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
