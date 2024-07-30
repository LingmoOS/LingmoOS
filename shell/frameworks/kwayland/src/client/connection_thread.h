/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_CONNECTION_THREAD_H
#define WAYLAND_CONNECTION_THREAD_H

#include <QList>
#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_display;

namespace KWayland
{
/**
 * @short KWayland Client.
 *
 * This namespace groups all classes related to the Client module.
 *
 * The main entry point into the KWayland::Client API is the ConnectionThread class.
 * It allows to create a Wayland client connection either in a native way or wrap a
 * connection created by the QtWayland QPA plugin.
 *
 * KWayland::Client provides one the one hand a low-level API to interact with the
 * Wayland API, on the other hand an easy to use convenience API. Each class directly
 * relates to a low-level Wayland type and allows direct casting into the type.
 *
 * On the convenience side KWayland::Client allows easy creation of objects, signals
 * emitted for Wayland events and easy conversion from Qt to Wayland types.
 *
 * Once one has a ConnectionThread created, it's possible to setup a Registry to
 * get a listing of all registered globals. For each global the Registry provides a convenience
 * method to create the resource.
 *
 * @see ConnectionThread
 * @see Registry
 **/
namespace Client
{
/**
 * @short Creates and manages the connection to a Wayland server.
 *
 * The purpose of this class is to create the connection to a Wayland server
 * and to manage it. As the name suggests it's intended to move instances of
 * this class into a dedicated thread. This also means that this class doesn't
 * inherit QThread. In order to use it in a threaded way one needs to create a
 * QThread and move the object there:
 *
 * @code
 * ConnectionThread *connection = new ConnectionThread;
 * QThread *thread = new QThread;
 * connection->moveToThread(thread);
 * thread->start();
 * @endcode
 *
 * To finalize the initialization of the connection one needs to call @link ::initConnection @endlink.
 * This starts an asynchronous connection initialization. In case the initialization
 * succeeds the signal @link ::connected @endlink will be emitted, otherwise @link ::failed @endlink will
 * be emitted:
 *
 * @code
 * connect(connection, &ConnectionThread::connected, [connection] {
 *     qDebug() << "Successfully connected to Wayland server at socket:" << connection->socketName();
 * });
 * connect(connection, &ConnectionThread::failed, [connection] {
 *     qDebug() << "Failed to connect to Wayland server at socket:" << connection->socketName();
 * });
 * connection->initConnection();
 * @endcode
 *
 * This class is also responsible for dispatching events. Whenever new data is available on
 * the Wayland socket, it will be dispatched and the signal @link ::eventsRead @endlink is emitted.
 * This allows further event queues in other threads to also dispatch their events.
 *
 * Furthermore this class flushes the Wayland connection whenever the QAbstractEventDispatcher
 * is about to block.
 *
 * To disconnect the connection to the Wayland server one should delete the instance of this
 * class and quit the dedicated thread:
 *
 * @code
 * connection->deleteLater();
 * thread->quit();
 * thread->wait();
 * @endcode
 *
 * In addition the ConnectionThread provides integration with QtWayland QPA plugin. For that
 * it provides a static factory method:
 *
 * @code
 * auto connection = ConnectionThread::fromApplication();
 * @endcode
 *
 * The semantics of the ConnectionThread are slightly changed if it's integrated with QtWayland.
 * The ConnectionThread does not hold the connection, does not emit connected or released signals
 * (one can safely assume that the connection is valid when integrating with the Qt application),
 * does not dispatch events. Given that the use case of the ConnectionThread is rather limited to
 * a convenient API around wl_display to allow easily setup an own Registry in a QtWayland powered
 * application. Also moving the ConnectionThread to a different thread is not necessarily recommended
 * in that case as QtWayland holds it's connection in an own thread anyway.
 *
 **/
class KWAYLANDCLIENT_EXPORT ConnectionThread : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionThread(QObject *parent = nullptr);
    ~ConnectionThread() override;

    /**
     * Creates a ConnectionThread for the used QGuiApplication.
     * This is an integration feature for QtWayland. On non-wayland platforms this method returns
     * @c nullptr.
     *
     * The returned ConnectionThread will be fully setup, which means it manages a wl_display.
     * There is no need to initConnection and the connected or failed signals won't be emitted.
     * When the created ConnectionThread gets destroyed the managed wl_display won't be disconnected
     * as that's managed by Qt.
     *
     * The returned ConnectionThread is not able to detect (protocol) error. The signal
     * {@link errorOccurred} won't be emitted, {@link hasError} will return @c false, even if the
     * actual connection held by QtWayland is on error. The behavior of QtWayland is to exit the
     * application on error.
     *
     * @since 5.4
     **/
    static ConnectionThread *fromApplication(QObject *parent = nullptr);

    /**
     * The display this ConnectionThread is connected to.
     * As long as there is no connection this method returns @c null.
     * @see initConnection
     **/
    wl_display *display();
    /**
     * @returns the name of the socket it connects to.
     **/
    QString socketName() const;
    /**
     * Sets the @p socketName to connect to.
     * Only applies if called before calling initConnection.
     * The default socket name is derived from environment variable WAYLAND_DISPLAY
     * and if not set is hard coded to "wayland-0".
     *
     * The socket name will be ignored if a file descriptor has been set through @link setSocketFd @endlink.
     *
     * @see setSocketFd
     **/
    void setSocketName(const QString &socketName);
    /**
     * Sets the socket @p fd to connect to.
     * Only applies if called before calling initConnection.
     * If this method is invoked, the connection will be created on the file descriptor
     * and not on the socket name passed through @link setSocketName @endlink or through the
     * default environment variable WAYLAND_DISPLAY.
     * @see setSocketName
     **/
    void setSocketFd(int fd);

    /**
     * Trigger a blocking roundtrip to the Wayland server. Ensures that all events are processed
     * before returning to the event loop.
     *
     * @since 5.4
     **/
    void roundtrip();

    /**
     * @returns whether the Wayland connection experienced an error
     * @see errorCode
     * @see errorOccurred
     * @since 5.23
     **/
    bool hasError() const;

    /**
     * @returns the error code of the last occurred error or @c 0 if the connection doesn't have an error
     * @see hasError
     * @see errorOccurred
     * @since 5.23
     **/
    int errorCode() const;

    /**
     * @returns all connections created in this application
     * @since 5.37
     **/
    static QList<ConnectionThread *> connections();

public Q_SLOTS:
    /**
     * Initializes the connection in an asynchronous way.
     * In case the connection gets established the signal @link ::connected @endlink will be
     * emitted, on failure the signal @link ::failed @endlink will be emitted.
     *
     * @see connected
     * @see failed
     **/
    void initConnection();

    /**
     * Explicitly flush the Wayland display.
     * @since 5.3
     **/
    void flush();

Q_SIGNALS:
    /**
     * Emitted once a connection to a Wayland server is established.
     * Normally emitted after invoking initConnection(), but might also be
     * emitted after re-connecting to another server.
     **/
    void connected();
    /**
     * Emitted if connecting to a Wayland server failed.
     **/
    void failed();
    /**
     * Emitted whenever new events are ready to be read.
     **/
    void eventsRead();
    /**
     * Emitted if the Wayland server connection dies.
     * If the socket reappears, it is tried to reconnect.
     **/
    void connectionDied();
    /**
     * The Wayland connection experienced a fatal error.
     * The ConnectionThread is no longer valid, no requests may be sent.
     * This has the same effects as {@link connectionDied}.
     *
     * @see hasError
     * @see errorCode
     * @since 5.23
     **/
    void errorOccurred();

protected:
    /*
     * Creates a connection thread from an existing wl_display object
     * @see ConnectionThread::fromApplication
     */
    explicit ConnectionThread(wl_display *display, QObject *parent);

private Q_SLOTS:
    /**
     * @internal
     **/
    void doInitConnection();

private:
    class Private;
    QScopedPointer<Private> d;
};
}
}

#endif
