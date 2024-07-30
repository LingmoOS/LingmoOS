/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_TOUCH_H
#define WAYLAND_TOUCH_H

#include <QObject>
#include <QPoint>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_touch;

namespace KWayland
{
namespace Client
{
class Surface;
class Touch;

/**
 * TODO
 */
class KWAYLANDCLIENT_EXPORT TouchPoint
{
public:
    virtual ~TouchPoint();

    /**
     * Unique in the scope of all TouchPoints currently being down.
     * As soon as the TouchPoint is now longer down another TouchPoint
     * might get assigned the id.
     **/
    qint32 id() const;
    /**
     * The serial when the down event happened.
     **/
    quint32 downSerial() const;
    /**
     * The serial when the up event happened.
     **/
    quint32 upSerial() const;
    /**
     * Most recent timestamp
     **/
    quint32 time() const;
    /**
     * All timestamps, references the positions.
     * That is each position has a timestamp.
     **/
    QList<quint32> timestamps() const;
    /**
     * Most recent position
     **/
    QPointF position() const;
    /**
     * All positions this TouchPoint had, updated with each move.
     **/
    QList<QPointF> positions() const;
    /**
     * The Surface this TouchPoint happened on.
     **/
    QPointer<Surface> surface() const;
    /**
     * @c true if currently down, @c false otherwise.
     **/
    bool isDown() const;

private:
    friend class Touch;
    explicit TouchPoint();
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the wl_touch interface.
 *
 * This class is a convenient wrapper for the wl_touch interface.
 *
 * To create an instance use Seat::createTouch.
 *
 * @see Seat
 **/
class KWAYLANDCLIENT_EXPORT Touch : public QObject
{
    Q_OBJECT
public:
    explicit Touch(QObject *parent = nullptr);
    ~Touch() override;

    /**
     * @returns @c true if managing a wl_pointer.
     **/
    bool isValid() const;
    /**
     * Setup this Touch to manage the @p touch.
     * When using Seat::createTouch there is no need to call this
     * method.
     **/
    void setup(wl_touch *touch);
    /**
     * Releases the wl_touch interface.
     * After the interface has been released the Touch instance is no
     * longer valid and can be setup with another wl_touch interface.
     *
     * This method is automatically invoked when the Seat which created this
     * Touch gets released.
     **/
    void release();
    /**
     * Destroys the data held by this Touch.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_touch interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Seat which created this
     * Touch gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * The TouchPoints of the latest touch event sequence.
     * Only valid till the next touch event sequence is started
     **/
    QList<TouchPoint *> sequence() const;

    operator wl_touch *();
    operator wl_touch *() const;

Q_SIGNALS:
    /**
     * A new touch sequence is started. The previous sequence is discarded.
     * @param startPoint The first point which started the sequence
     **/
    void sequenceStarted(KWayland::Client::TouchPoint *startPoint);
    /**
     * Sent if the compositor decides the touch stream is a global
     * gesture.
     **/
    void sequenceCanceled();
    /**
     * Emitted once all touch points are no longer down.
     **/
    void sequenceEnded();
    /**
     * Indicates the end of a contact point list.
     **/
    void frameEnded();
    /**
     * TouchPoint @p point got added to the sequence.
     **/
    void pointAdded(KWayland::Client::TouchPoint *point);
    /**
     * TouchPoint @p point is no longer down.
     * A new TouchPoint might reuse the Id of the @p point.
     **/
    void pointRemoved(KWayland::Client::TouchPoint *point);
    /**
     * TouchPoint @p point moved.
     **/
    void pointMoved(KWayland::Client::TouchPoint *point);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::TouchPoint *)

#endif
