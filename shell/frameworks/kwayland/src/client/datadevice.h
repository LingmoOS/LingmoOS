/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_DATADEVICE_H
#define WAYLAND_DATADEVICE_H

#include "dataoffer.h"

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_data_device;

namespace KWayland
{
namespace Client
{
class DataSource;
class Surface;

/**
 * @short DataDevice allows clients to share data by copy-and-paste and drag-and-drop.
 *
 * This class is a convenient wrapper for the wl_data_device interface.
 * To create a DataDevice call DataDeviceManager::getDataDevice.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataDevice : public QObject
{
    Q_OBJECT
public:
    explicit DataDevice(QObject *parent = nullptr);
    ~DataDevice() override;

    /**
     * Setup this DataDevice to manage the @p dataDevice.
     * When using DataDeviceManager::createDataDevice there is no need to call this
     * method.
     **/
    void setup(wl_data_device *dataDevice);
    /**
     * Releases the wl_data_device interface.
     * After the interface has been released the DataDevice instance is no
     * longer valid and can be setup with another wl_data_device interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataDevice.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_device interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataDevice gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_device.
     **/
    bool isValid() const;

    void startDrag(quint32 serial, DataSource *source, Surface *origin, Surface *icon = nullptr);
    void startDragInternally(quint32 serial, Surface *origin, Surface *icon = nullptr);

    void setSelection(quint32 serial, DataSource *source = nullptr);
    void clearSelection(quint32 serial);

    DataOffer *offeredSelection() const;

    /**
     * @returns the currently focused surface during drag'n'drop on this DataDevice.
     * @since 5.22
     **/
    QPointer<Surface> dragSurface() const;
    /**
     * @returns the DataOffer during a drag'n'drop operation.
     * @since 5.22
     **/
    DataOffer *dragOffer() const;

    operator wl_data_device *();
    operator wl_data_device *() const;

Q_SIGNALS:
    void selectionOffered(KWayland::Client::DataOffer *);
    void selectionCleared();
    /**
     * Notification that a drag'n'drop operation entered a Surface on this DataDevice.
     *
     * @param serial The serial for this enter
     * @param relativeToSurface Coordinates relative to the upper-left corner of the Surface.
     * @see dragSurface
     * @see dragOffer
     * @see dragLeft
     * @see dragMotion
     * @since 5.22
     **/
    void dragEntered(quint32 serial, const QPointF &relativeToSurface);
    /**
     * Notification that the drag'n'drop operation left the Surface on this DataDevice.
     *
     * The leave notification is sent before the enter notification for the new focus.
     * @see dragEnter
     * @since 5.22
     **/
    void dragLeft();
    /**
     * Notification of drag motion events on the current drag surface.
     *
     * @param relativeToSurface  Coordinates relative to the upper-left corner of the entered Surface.
     * @param time timestamp with millisecond granularity
     * @see dragEntered
     * @since 5.22
     **/
    void dragMotion(const QPointF &relativeToSurface, quint32 time);
    /**
     * Emitted when the implicit grab is removed and the drag'n'drop operation ended on this
     * DataDevice.
     *
     * The client can now start a data transfer on the DataOffer.
     * @see dragEntered
     * @see dragOffer
     * @since 5.22
     **/
    void dropped();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
