// Copyright 2021  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef WAYLAND_DDE_KEYBOARD_H
#define WAYLAND_DDE_KEYBOARD_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QVector>

#include <DWayland/Client/kwaylandclient_export.h>

struct dde_seat;
struct dde_keyboard;
class  QPoint;
class  QRect;

namespace KWayland
{
namespace Client
{

/**
 * @short Wrapper for the dde_keyboard interface.
 *
 * This class is a convenient wrapper for the dde_keyboard interface.
 *
 * To create an instance use DDEKeyboard::createDDEKeyboard.
 *
 *
 * @see DDEKeyboard
 **/
class KWAYLANDCLIENT_EXPORT DDEKeyboard : public QObject
{
    Q_OBJECT
public:
    enum class KeyState {
        Released,
        Pressed
    };
    explicit DDEKeyboard(QObject *parent = nullptr);
    virtual ~DDEKeyboard();

    /**
     * @returns @c true if managing a dde_keyboard.
     **/
    bool isValid() const;
    /**
     * Setup this DDEKeyboard to manage the @p keyboard.
     * When using Seat::createKeyboard there is no need to call this
     * method.
     **/
    void setup(dde_keyboard *keyboard);
    /**
     * Releases the dde_keyboard interface.
     * After the interface has been released the DDEKeyboard instance is no
     * longer valid and can be setup with another dde_keyboard interface.
     *
     * This method is automatically invoked when the Seat which created this
     * DDEKeyboard gets released.
     **/
    void release();
    /**
     * Destroys the data held by this DDEKeyboard.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new dde_keyboard interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Seat which created this
     * DDEKeyboard gets destroyed.
     *
     * @see release
     **/
    void destroy();

    operator dde_keyboard*();
    operator dde_keyboard*() const;

Q_SIGNALS:
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
    void keyChanged(quint32 key, KWayland::Client::DDEKeyboard::KeyState state, quint32 time);
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

Q_DECLARE_METATYPE(KWayland::Client::DDEKeyboard::KeyState)

#endif