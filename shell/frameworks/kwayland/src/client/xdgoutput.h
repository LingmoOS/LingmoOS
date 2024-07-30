/*
    SPDX-FileCopyrightText: 2018 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_XDGOUTPUT_H
#define KWAYLAND_CLIENT_XDGOUTPUT_H

#include <QObject>
#include <QPoint>
#include <QSize>

#include "KWayland/Client/kwaylandclient_export.h"

struct zxdg_output_manager_v1;
struct zxdg_output_v1;

namespace KWayland
{
namespace Client
{
class EventQueue;
class XdgOutput;
class Output;

/**
 * @short Wrapper for the zxdg_output_manager_v1 interface.
 *
 * This class provides a convenient wrapper for the zxdg_output_manager_v1 interface.
 *
 * This provides the logical size of the output. This is useful in case it doesn't match the
 * pixelSize / outputScale.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the XdgOutputManager interface:
 * @code
 * XdgOutputManager *c = registry->createXdgOutputManager(name, version);
 * @endcode
 *
 * This creates the XdgOutputManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * XdgOutputManager *c = new XdgOutputManager;
 * c->setup(registry->bindXdgOutputManager(name, version));
 * @endcode
 *
 * The XdgOutputManager can be used as a drop-in replacement for any zxdg_output_manager_v1
 * pointer as it provides matching cast operators.
 *
 * @since 5.47
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT XdgOutputManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new XdgOutputManager.
     * Note: after constructing the XdgOutputManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use XdgOutputManager prefer using
     * Registry::createXdgOutputManager.
     **/
    explicit XdgOutputManager(QObject *parent = nullptr);
    ~XdgOutputManager() override;

    /**
     * Setup this XdgOutputManager to manage the @p xdgoutputmanager.
     * When using Registry::createXdgOutputManager there is no need to call this
     * method.
     **/
    void setup(zxdg_output_manager_v1 *xdgoutputmanager);
    /**
     * @returns @c true if managing a zxdg_output_manager_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_output_manager_v1 interface.
     * After the interface has been released the XdgOutputManager instance is no
     * longer valid and can be setup with another zxdg_output_manager_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgOutputManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_output_manager_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgoutputmanager, &XdgOutputManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this XdgOutputManager.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this XdgOutputManager.
     **/
    EventQueue *eventQueue();

    XdgOutput *getXdgOutput(Output *output, QObject *parent = nullptr);

    operator zxdg_output_manager_v1 *();
    operator zxdg_output_manager_v1 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the XdgOutputManager got created by
     * Registry::createXdgOutputManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the zxdg_output_v1 interface.
 *
 * This class provides a convenient wrapper for the zxdg_output_v1 interface.
 *
 * The XdgOutputManager can be used as a drop-in replacement for any zxdg_output_v1
 * pointer as it provides matching cast operators.
 *
 * This protocol provides a potentially more correct size and position of the screen
 * than Output with respect to scaling.
 *
 * @see Registry
 **/

class KWAYLANDCLIENT_EXPORT XdgOutput : public QObject
{
    Q_OBJECT
public:
    ~XdgOutput() override;

    /**
     * Setup this XdgOutput to manage the @p xdgoutput.
     * When using XdgOutputManager::createXdgOutput there is no need to call this
     * method.
     **/
    void setup(zxdg_output_v1 *xdgoutput);
    /**
     * @returns @c true if managing a zxdg_output_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_output_v1 interface.
     * After the interface has been released the XdgOutput instance is no
     * longer valid and can be setup with another zxdg_output_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgOutput.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_output_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgoutput, &XdgOutput::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    operator zxdg_output_v1 *();
    operator zxdg_output_v1 *() const;

    /**
     * The top left position of the output in compositor coordinates
     */
    QPoint logicalPosition() const;

    /**
     * The size of the output in compositor coordinates
     * (i.e pixel size / output scale)
     */
    QSize logicalSize() const;

    /**
     * A consistent unique name for this monitor
     * @since 5.XDGOUTPUT
     */
    QString name() const;

    /**
     * A longer human readable description
     * @since 5.XDGOUTPUT
     */
    QString description() const;

Q_SIGNALS:
    /**
     * Emitted when any of the attributes have changed
     */
    void changed();

private:
    friend class XdgOutputManager;
    explicit XdgOutput(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
