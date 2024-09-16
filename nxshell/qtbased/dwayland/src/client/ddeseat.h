// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef WAYLAND_DDE_SEAT_H
#define WAYLAND_DDE_SEAT_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QVector>

#include <DWayland/Client/kwaylandclient_export.h>

struct dde_seat;
struct dde_pointer;
struct dde_touch;
class  QPoint;
class  QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;
class DDEPointer;
class DDEKeyboard;
class DDETouch;

/**
 * @short Wrapper for the dde_seat interface.
 *
 * This class provides a convenient wrapper for the dde_seat interface.
 * Its main purpose is to hold the information about one DDESeat.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an DDESeat interface:
 * @code
 * DDESeat *c = registry->createDDESeat(name, version);
 * @endcode
 *
 * This creates the DDESeat and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DDESeat *c = new DDESeat;
 * c->setup(registry->bindDDESeat(name, version));
 * @endcode
 *
 * The DDESeat can be used as a drop-in replacement for any dde_seat
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of DDESeat are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the DDESeat instance. By emitting changed
 * the DDESeat indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT DDESeat : public QObject
{
    Q_OBJECT
public:

    explicit DDESeat(QObject *parent = nullptr);
    virtual ~DDESeat();

    /**
     * Setup this Compositor to manage the @p DDESeat.
     * When using Registry::createDDESeat there is no need to call this
     * method.
     **/
    void setup(dde_seat *DDESeat);

    /**
     * @returns @c true if managing a dde_seat.
     **/
    bool isValid() const;
    operator dde_seat*();
    operator dde_seat*() const;

    /**
     * Creates a DDEPointer and sets it up.
     *
     *
     * @param parent The parent to use for the DDEPointer
     * @returns created DDEPointer
     **/
    DDEPointer *createDDePointer(QObject *parent = nullptr);

    /**
     * Creates a DDEKeyboard and sets it up.
     *
     *
     * @param parent The parent to use for the DDEKeyboard
     * @returns created DDEKeyboard
     **/
    DDEKeyboard *createDDEKeyboard(QObject *parent = nullptr);

    /**
     * Creates a DDETouch and sets it up.
     *
     *
     * @param parent The parent to use for the DDETouch
     * @returns created DDETouch
     **/
    DDETouch *createDDETouch(QObject *parent = nullptr);

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
    * Destroys the data hold by this DDESeat.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * DDESeat gets destroyed.
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
     * This signal gets only emitted if the DDESeat got created by
     * Registry::createDDESeat
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the dde_pointer interface.
 *
 * This class is a convenient wrapper for the dde_pointer interface.
 *
 * To create an instance use DDESeat::createDDePointer.
 *
 *
 * @see DDESeat
 **/
class KWAYLANDCLIENT_EXPORT DDEPointer : public QObject
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

    explicit DDEPointer(QObject *parent);
    virtual ~DDEPointer();

    /**
     * Releases the dde_pointer interface.
     * After the interface has been released the DDEPointer instance is no
     * longer valid and can be setup with another dde_pointer interface.
     *
     * This method is automatically invoked when the DDESeat which created this
     * DDEPointer gets released.
     **/
    void release();
    /**
     * Destroys the data held by this DDEPointer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new dde_pointer interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the DDESeat which created this
     * DDEPointer gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this DDEPointer to manage the @p dde_pointer.
     * There is normally no need to call this method as it's invoked by
     * DDESeat::createDDEPointer.
     **/
    void setup(dde_pointer *ddePointer);

    /**
     * @returns @c true if managing a dde_pointer.
     **/
    bool isValid() const;
    operator dde_pointer*();
    operator dde_pointer*() const;

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
    void buttonStateChanged(const QPointF &globalPos, quint32 button, KWayland::Client::DDEPointer::ButtonState state);
    /**
     * Scroll and other axis notifications.
     *
     * @param time timestamp with millisecond granularity
     * @param axis @c Vertical or @c Horizontal
     * @param delta
     **/
    void axisChanged(quint32 time, KWayland::Client::DDEPointer::Axis axis, qreal delta);

private:
    friend class DDESeat;
    class Private;
    QScopedPointer<Private> d;
};


/**
 * @short Wrapper for the dde_touch interface.
 *
 * This class is a convenient wrapper for the dde_touch interface.
 *
 * To create an instance use DDETouch::createDDETouch.
 *
 *
 * @see DDETouch
 **/
class KWAYLANDCLIENT_EXPORT DDETouch : public QObject
{
    Q_OBJECT
public:
    explicit DDETouch(QObject *parent = nullptr);
    virtual ~DDETouch();

    /**
     * @returns @c true if managing a dde_touch.
     **/
    bool isValid() const;
    /**
     * Setup this DDETouch to manage the @p touch.
     * When using Seat::createDDETouch there is no need to call this
     * method.
     **/
    void setup(dde_touch *touch);
    /**
     * Releases the dde_touch interface.
     * After the interface has been released the DDETouch instance is no
     * longer valid and can be setup with another dde_touch interface.
     *
     * This method is automatically invoked when the Seat which created this
     * DDEKeyboard gets released.
     **/
    void release();
    /**
     * Destroys the data held by this DDETouch.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new dde_touch interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Seat which created this
     * DDETouch gets destroyed.
     *
     * @see release
     **/
    void destroy();

    operator dde_touch*();
    operator dde_touch*() const;

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

Q_DECLARE_METATYPE(KWayland::Client::DDEPointer::ButtonState)

#endif
