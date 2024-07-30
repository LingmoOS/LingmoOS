/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_REGION_H
#define WAYLAND_REGION_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_region;

namespace KWayland
{
namespace Client
{
/**
 * @short Wrapper for the wl_region interface.
 *
 * This class is a convenient wrapper for the wl_region interface.
 * To create a Region call Compositor::createRegion.
 *
 * The main purpose of this class is to provide regions which can be
 * used to e.g. set the input region on a Surface.
 *
 * @see Compositor
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT Region : public QObject
{
    Q_OBJECT
public:
    explicit Region(const QRegion &region, QObject *parent = nullptr);
    ~Region() override;

    /**
     * Setup this Surface to manage the @p region.
     * When using Compositor::createRegion there is no need to call this
     * method.
     **/
    void setup(wl_region *region);
    /**
     * Releases the wl_region interface.
     * After the interface has been released the Region instance is no
     * longer valid and can be setup with another wl_region interface.
     **/
    void release();
    /**
     * Destroys the data held by this Region.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_region interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, region, &Region::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_region.
     **/
    bool isValid() const;

    /**
     * Adds the @p rect to this Region.
     **/
    void add(const QRect &rect);
    /**
     * Adds the @p region to this Rregion.
     **/
    void add(const QRegion &region);
    /**
     * Subtracts @p rect from this Region.
     **/
    void subtract(const QRect &rect);
    /**
     * Subtracts @p region from this Region.
     **/
    void subtract(const QRegion &region);

    /**
     * The geometry of this Region.
     **/
    QRegion region() const;

    operator wl_region *();
    operator wl_region *() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
