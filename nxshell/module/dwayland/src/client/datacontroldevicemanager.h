/*
    SPDX-FileCopyrightText: 2021 TangHaixiang <tanghaixiang@unionteh.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/
#ifndef WAYLAND_DATA_CONTROL_DEVICE_MANAGER_H
#define WAYLAND_DATA_CONTROL_DEVICE_MANAGER_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>


struct zwlr_data_control_manager_v1;

namespace KWayland
{
namespace Client
{

class EventQueue;
class DataControlSourceV1;
class DataControlDeviceV1;
class Seat;

/**
 * @short Wrapper for the wl_data_device_manager interface.
 *
 * This class provides a convenient wrapper for the wl_data_device_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the DataDeviceManager interface:
 * @code
 * DataDeviceManager *m = registry->createDataDeviceManager(name, version);
 * @endcode
 *
 * This creates the DataDeviceManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DataDeviceManager *m = new DataDeviceManager;
 * m->setup(registry->bindDataDeviceManager(name, version));
 * @endcode
 *
 * The DataDeviceManager can be used as a drop-in replacement for any wl_data_device_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT DataControlDeviceManager : public QObject
{
    Q_OBJECT
public:

    /**
     * Creates a new Compositor.
     * Note: after constructing the Compositor it is not yet valid and one needs
     * to call setup. In order to get a ready to use Compositor prefer using
     * Registry::createCompositor.
     **/
    explicit DataControlDeviceManager(QObject *parent = nullptr);
    virtual ~DataControlDeviceManager();

    /**
     * @returns @c true if managing a wl_data_device_manager.
     **/
    bool isValid() const;
    /**
     * Setup this DataDeviceManager to manage the @p manager.
     * When using Registry::createDataDeviceManager there is no need to call this
     * method.
     **/
    void setup(zwlr_data_control_manager_v1 *manager);
    /**
     * Releases the wl_data_device_manager interface.
     * After the interface has been released the DataDeviceManager instance is no
     * longer valid and can be setup with another wl_data_device_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataDeviceManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_device_manager interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataDeviceManager gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a DataSource.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a DataSource.
     **/
    EventQueue *eventQueue();

    DataControlSourceV1 *createDataSource(QObject *parent = nullptr);

    DataControlDeviceV1 *getDataDevice(Seat *seat, QObject *parent = nullptr);

    operator zwlr_data_control_manager_v1*();
    operator zwlr_data_control_manager_v1*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createDataDeviceManager
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
