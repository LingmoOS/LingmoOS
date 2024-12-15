// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef WAYLAND_OCEAN_SEAT_H
#define WAYLAND_OCEAN_SEAT_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QVector>

#include <DWayland/Client/kwaylandclient_export.h>

struct ocean_seat;
struct ocean_pointer;
struct ocean_touch;
class  QPoint;
class  QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;
class OCEANPointer;
class OCEANKeyboard;
class OCEANTouch;

/**
 * @short Wrapper for the ocean_seat interface.
 *
 * This class provides a convenient wrapper for the ocean_seat interface.
 * Its main purpose is to hold the information about one OCEANSeat.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an OCEANSeat interface:
 * @code
 * OCEANSeat *c = registry->createOCEANSeat(name, version);
 * @endcode
 *
 * This creates the OCEANSeat and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * OCEANSeat *c = new OCEANSeat;
 * c->setup(registry->bindOCEANSeat(name, version));
 * @endcode
 *
 * The OCEANSeat can be used as a drop-in replacement for any ocean_seat
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of OCEANSeat are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the OCEANSeat instance. By emitting changed
 * the OCEANSeat indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT OCEANSeat : public QObject
{
    Q_OBJECT
public:

    explicit OCEANSeat(QObject *parent = nullptr);
    virtual ~OCEANSeat();

    /**
     * Setup this Compositor to manage the @p OCEANSeat.
     * When using Registry::createOCEANSeat there is no need to call this
     * method.
     **/
    void setup(ocean_seat *OCEANSeat);

    /**
     * @returns @c true if managing a ocean_seat.
     **/
    bool isValid() const;
    operator ocean_seat*();
    operator ocean_seat*() const;

    /**
     * Creates a OCEANPointer and sets it up.
     *
     *
     * @param parent The parent to use for the OCEANPointer
     * @returns created OCEANPointer
     **/
    OCEANPointer *createDDePointer(QObject *parent = nullptr);

    /**
     * Creates a OCEANKeyboard and sets it up.
     *
     *
     * @param parent The parent to use for the OCEANKeyboard
     * @returns created OCEANKeyboard
     **/
    OCEANKeyboard *createOCEANKeyboard(QObject *parent = nullptr);

    /**
     * Creates a OCEANTouch and sets it up.
     *
     *
     * @param parent The parent to use for the OCEANTouch
     * @returns created OCEANTouch
     **/
    OCEANTouch *createOCEANTouch(QObject *parent = nullptr);

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    void release();
    /**
    * Destroys the data hold by this OCEANSeat.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * OCEANSeat gets destroyed.
    **/
    void destroy();

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the OCEANSeat got created by
     * Registry::createOCEANSeat
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the ocean_pointer interface.
 *
 * This class is a convenient wrapper for the ocean_pointer interface.
 *
 * To create an instance use OCEANSeat::createDDePointer.
 *
 *
 * @see OCEANSeat
 **/
class KWAYLANDCLIENT_EXPORT OCEANPointer : public QObject
{
    Q_OBJECT
public:
    enum class ButtonState {
        Released,
        Pressed
    };
    enum class Axis {
        Vertical,
        Horizontal
    };

    explicit OCEANPointer(QObject *parent);
    virtual ~OCEANPointer();

    /**
     * Releases the ocean_pointer interface.
     * After the interface has been released the OCEANPointer instance is no
     * longer valid and can be setup with another ocean_pointer interface.
     *
     * This method is automatically invoked when the OCEANSeat which created this
     * OCEANPointer gets released.
     **/
    void release();
    /**
     * Destroys the data held by this OCEANPointer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new ocean_pointer interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the OCEANSeat which created this
     * OCEANPointer gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this OCEANPointer to manage the @p ocean_pointer.
     * There is normally no need to call this method as it's invoked by
     * OCEANSeat::createOCEANPointer.
     **/
    void setup(ocean_pointer *oceanPointer);

    /**
     * @returns @c true if managing a ocean_pointer.
     **/
    bool isValid() const;
    operator ocean_pointer*();
    operator ocean_pointer*() const;

    void getMotion();
    QPointF getGlobalPointerPos();

Q_SIGNALS:
    /**
     * Notification of pointer location change.
     *
     * @param globalPos  the global cursor position.
     **/
    void motion(const QPointF &globalPos);
    /**
     * Mouse button click and release notifications.
     *
     * The location of the click is given by the last motion or enter event.
     *
     * @param globalPos the global cursor position.
     * @param button The button which got changed
     * @param state @c Released or @c Pressed
     **/
    void buttonStateChanged(const QPointF &globalPos, quint32 button, KWayland::Client::OCEANPointer::ButtonState state);
    /**
     * Scroll and other axis notifications.
     *
     * @param time timestamp with millisecond granularity
     * @param axis @c Vertical or @c Horizontal
     * @param delta
     **/
    void axisChanged(quint32 time, KWayland::Client::OCEANPointer::Axis axis, qreal delta);

private:
    friend class OCEANSeat;
    class Private;
    QScopedPointer<Private> d;
};


/**
 * @short Wrapper for the ocean_touch interface.
 *
 * This class is a convenient wrapper for the ocean_touch interface.
 *
 * To create an instance use OCEANTouch::createOCEANTouch.
 *
 *
 * @see OCEANTouch
 **/
class KWAYLANDCLIENT_EXPORT OCEANTouch : public QObject
{
    Q_OBJECT
public:
    explicit OCEANTouch(QObject *parent = nullptr);
    virtual ~OCEANTouch();

    /**
     * @returns @c true if managing a ocean_touch.
     **/
    bool isValid() const;
    /**
     * Setup this OCEANTouch to manage the @p touch.
     * When using Seat::createOCEANTouch there is no need to call this
     * method.
     **/
    void setup(ocean_touch *touch);
    /**
     * Releases the ocean_touch interface.
     * After the interface has been released the OCEANTouch instance is no
     * longer valid and can be setup with another ocean_touch interface.
     *
     * This method is automatically invoked when the Seat which created this
     * OCEANKeyboard gets released.
     **/
    void release();
    /**
     * Destroys the data held by this OCEANTouch.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new ocean_touch interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Seat which created this
     * OCEANTouch gets destroyed.
     *
     * @see release
     **/
    void destroy();

    operator ocean_touch*();
    operator ocean_touch*() const;

Q_SIGNALS:
    /**
     * This signal provides a file descriptor to the client which can
     * be memory-mapped to provide a keyboard mapping description.
     *
     * The signal is only emitted if the keymap format is libxkbcommon compatible.
     *
     * @param id id of the touch
     * @param pos The position of the touch
     **/
    void touchDown(int32_t id, const QPointF &pos);
    /**
     * A key was pressed or released.
     * The time argument is a timestamp with millisecond granularity, with an undefined base.
     * @param id id of the touch
     * @param pos The position of the touch
     **/
    void touchMotion(int32_t id, const QPointF &pos);
    /**
     * Notifies clients that the modifier and/or group state has changed,
     * and it should update its local state.
     * @param id id of the touch
     **/
    void touchUp(int32_t id);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::OCEANPointer::ButtonState)

#endif
