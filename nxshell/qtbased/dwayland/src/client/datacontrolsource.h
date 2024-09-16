/*
    SPDX-FileCopyrightText: 2021 TangHaixiang <tanghaixiang@unionteh.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/
#ifndef WAYLAND_DATA_CONTROL_SOURCE_H
#define WAYLAND_DATA_CONTROL_SOURCE_H

#include "buffer.h"
#include "datacontroldevicemanager.h"

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

class QMimeType;

struct zwlr_data_control_source_v1;

namespace KWayland
{
namespace Client
{



/**
 * @short Wrapper for the wl_data_source interface.
 *
 * This class is a convenient wrapper for the wl_data_source interface.
 * To create a DataSource call DataDeviceManager::createDataSource.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataControlSourceV1 : public QObject
{
    Q_OBJECT
public:
    explicit DataControlSourceV1(QObject *parent = nullptr);
    virtual ~DataControlSourceV1();

    /**
     * Setup this DataSource to manage the @p dataSource.
     * When using DataDeviceManager::createDataSource there is no need to call this
     * method.
     **/
    void setup(zwlr_data_control_source_v1 *dataSource);
    /**
     * Releases the wl_data_source interface.
     * After the interface has been released the DataSource instance is no
     * longer valid and can be setup with another wl_data_source interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataSource.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_source interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataSource gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_source.
     **/
    bool isValid() const;

    void offer(const QString &mimeType);
    void offer(const QMimeType &mimeType);


    operator zwlr_data_control_source_v1*();
    operator zwlr_data_control_source_v1*() const;

Q_SIGNALS:

    void sendDataRequested(const QString &mimeType, qint32 fd);
    /**
     * This DataSource has been replaced by another DataSource.
     * The client should clean up and destroy this DataSource.
     **/
    void cancelled();


private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
