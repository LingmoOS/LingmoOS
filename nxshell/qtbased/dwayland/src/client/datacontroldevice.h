/*
    SPDX-FileCopyrightText: 2021 TangHaixiang <tanghaixiang@unionteh.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/
#ifndef WAYLAND_DATA_CONTROL_DDEVICE_H
#define WAYLAND_DATA_CONTROL_DDEVICE_H

#include "dataoffer.h"

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct zwlr_data_control_device_v1;

namespace KWayland
{
namespace Client
{
class DataControlSourceV1;
class DataControlOfferV1;
class Surface;

/**
 * @short Wrapper for the wl_data_device interface.
 *
 * This class is a convenient wrapper for the wl_data_device interface.
 * To create a DataDevice call DataDeviceManager::getDataDevice.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataControlDeviceV1 : public QObject
{
    Q_OBJECT
public:
    explicit DataControlDeviceV1(QObject *parent = nullptr);
    virtual ~DataControlDeviceV1();

    /**
     * Setup this DataDevice to manage the @p dataDevice.
     * When using DataDeviceManager::createDataDevice there is no need to call this
     * method.
     **/
    void setup(zwlr_data_control_device_v1 *dataDevice);
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

    void setSelection(quint32 serial, DataControlSourceV1 *source = nullptr);
    void setCachedSelection(quint32 serial, DataControlSourceV1 *source = nullptr);
    void clearSelection(quint32 serial);

    DataControlOfferV1 *offeredSelection() const;


    operator zwlr_data_control_device_v1*();
    operator zwlr_data_control_device_v1*() const;

Q_SIGNALS:
    void selectionOffered(KWayland::Client::DataControlOfferV1*);
    void dataOffered(KWayland::Client::DataControlOfferV1*);
    void dataControlOffered(KWayland::Client::DataControlOfferV1*);
    void selectionCleared();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
