/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SEAT_H
#define WAYLAND_SEAT_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_seat;
struct wl_touch;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Keyboard;
class Pointer;
class Touch;

/**
 * @short Wrapper for the wl_seat interface.
 *
 * This class provides a convenient wrapper for the wl_seat interface.
 * It's main purpose is to provide the interfaces for Keyboard, Pointer and Touch.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Seat interface:
 * @code
 * Seat *s = registry->createSeat(name, version);
 * @endcode
 *
 * This creates the Seat and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * Seat *s = new Seat;
 * s->setup(registry->bindSeat(name, version));
 * @endcode
 *
 * The Seat can be used as a drop-in replacement for any wl_seat
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see Keyboard
 * @see Pointer
 **/
class KWAYLANDCLIENT_EXPORT Seat : public QObject
{
    Q_OBJECT
    /**
     * The seat has pointer devices. Default value is @c false.
     **/
    Q_PROPERTY(bool keyboard READ hasKeyboard NOTIFY hasKeyboardChanged)
    /**
     * The seat has pointer devices. Default value is @c false.
     **/
    Q_PROPERTY(bool pointer READ hasPointer NOTIFY hasPointerChanged)
    /**
     * The seat has touch devices. Default value is @c false.
     **/
    Q_PROPERTY(bool touch READ hasTouch NOTIFY hasTouchChanged)
    /**
     * In a multiseat configuration this can be used by the client to help identify
     * which physical devices the seat represents.
     * Based on the seat configuration used by the compositor.
     **/
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
public:
    explicit Seat(QObject *parent = nullptr);
    ~Seat() override;

    /**
     * @returns @c true if managing a wl_seat.
     **/
    bool isValid() const;
    /**
     * Setup this Seat to manage the @p seat.
     * When using Registry::createSeat there is no need to call this
     * method.
     **/
    void setup(wl_seat *seat);
    /**
     * Releases the wl_seat interface.
     * After the interface has been released the Seat instance is no
     * longer valid and can be setup with another wl_seat interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this Seat.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_shell interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * Seat gets destroyed.
     *
     * Right before the data is destroyed the signal interfaceAboutToBeDestroyed is emitted.
     *
     * @see release
     * @see interfaceAboutToBeDestroyed
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating Keyboard, Pointer and Touch.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating Keyboard, Pointer and Touch.
     **/
    EventQueue *eventQueue();

    bool hasKeyboard() const;
    bool hasPointer() const;
    bool hasTouch() const;
    QString name() const;
    operator wl_seat *();
    operator wl_seat *() const;

    /**
     * Creates a Keyboard.
     *
     * This method may only be called if the Seat has a keyboard.
     *
     * @param parent The parent to pass to the created Keyboard.
     * @returns The created Keyboard.
     **/
    Keyboard *createKeyboard(QObject *parent = nullptr);
    /**
     * Creates a Pointer.
     *
     * This method may only be called if the Seat has a pointer.
     *
     * @param parent The parent to pass to the created Pointer.
     * @returns The created Pointer.
     **/
    Pointer *createPointer(QObject *parent = nullptr);
    /**
     * Creates a Touch.
     *
     * This method may only be called if the Seat has touch support.
     *
     * @param parent The parent to pass to the created Touch.
     * @returns The created Touch.
     **/
    Touch *createTouch(QObject *parent = nullptr);

Q_SIGNALS:
    void hasKeyboardChanged(bool);
    void hasPointerChanged(bool);
    void hasTouchChanged(bool);
    void nameChanged(const QString &name);

    /**
     * This signal is emitted right before the interface is going to be released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is going to be destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createSeat
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
