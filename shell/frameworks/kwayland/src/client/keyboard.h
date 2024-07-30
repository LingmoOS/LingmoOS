/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_KEYBOARD_H
#define WAYLAND_KEYBOARD_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_keyboard;

namespace KWayland
{
namespace Client
{
class Surface;

/**
 * @short Wrapper for the wl_keyboard interface.
 *
 * This class is a convenient wrapper for the wl_keyboard interface.
 *
 * To create an instance use Seat::createKeyboard.
 *
 * @see Seat
 **/
class KWAYLANDCLIENT_EXPORT Keyboard : public QObject
{
    Q_OBJECT
public:
    enum class KeyState {
        Released,
        Pressed,
    };
    explicit Keyboard(QObject *parent = nullptr);
    ~Keyboard() override;

    /**
     * @returns @c true if managing a wl_keyboard.
     **/
    bool isValid() const;
    /**
     * Setup this Keyboard to manage the @p keyboard.
     * When using Seat::createKeyboard there is no need to call this
     * method.
     **/
    void setup(wl_keyboard *keyboard);
    /**
     * Releases the wl_keyboard interface.
     * After the interface has been released the Keyboard instance is no
     * longer valid and can be setup with another wl_keyboard interface.
     *
     * This method is automatically invoked when the Seat which created this
     * Keyboard gets released.
     **/
    void release();
    /**
     * Destroys the data held by this Keyboard.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_keyboard interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Seat which created this
     * Keyboard gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * @returns The Surface the Keyboard is on, may be @c null.
     **/
    Surface *enteredSurface() const;
    /**
     * @overload
     **/
    Surface *enteredSurface();

    /**
     * @returns Whether key repeat is enabled on this keyboard
     * @see keyRepeatRate
     * @see keyRepeatDelay
     * @see keyRepeatChanged
     * @since 5.5
     **/
    bool isKeyRepeatEnabled() const;
    /**
     * @returns the key repeat rate in characters per second.
     * @see isKeyRepeatEnabled
     * @see keyRepeatDelay
     * @see keyRepeatChanged
     * @since 5.5
     **/
    qint32 keyRepeatRate() const;
    /**
     * @returns the delay in millisecond for key repeat after a press.
     * @see isKeyRepeatEnabled
     * @see keyRepeatRate
     * @see keyRepeatChanged
     * @since 5.5
     **/
    qint32 keyRepeatDelay() const;

    operator wl_keyboard *();
    operator wl_keyboard *() const;

Q_SIGNALS:
    /**
     * Notification that this seat's Keyboard is focused on a certain surface.
     *
     * @param serial The serial for this enter
     **/
    void entered(quint32 serial);
    /**
     * Notification that this seat's Keyboard is no longer focused on a certain surface.
     *
     * The leave notification is sent before the enter notification for the new focus.
     *
     * @param serial The serial of this enter
     **/
    void left(quint32 serial);
    /**
     * This signal provides a file descriptor to the client which can
     * be memory-mapped to provide a keyboard mapping description.
     *
     * The signal is only emitted if the keymap format is libxkbcommon compatible.
     *
     * @param fd file descriptor of the keymap
     * @param size The size of the keymap
     **/
    void keymapChanged(int fd, quint32 size);
    /**
     * A key was pressed or released.
     * The time argument is a timestamp with millisecond granularity, with an undefined base.
     * @param key The key which was pressed
     * @param state Whether the key got @c Released or @c Pressed
     * @param time The timestamp
     **/
    void keyChanged(quint32 key, KWayland::Client::Keyboard::KeyState state, quint32 time);
    /**
     * Notifies clients that the modifier and/or group state has changed,
     * and it should update its local state.
     **/
    void modifiersChanged(quint32 depressed, quint32 latched, quint32 locked, quint32 group);
    /**
     * Emitted whenever information on key repeat changed.
     * @see isKeyRepeatEnabled
     * @see keyRepeatRate
     * @see keyRepeatDelay
     * @since 5.5
     **/
    void keyRepeatChanged();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Keyboard::KeyState)

#endif
