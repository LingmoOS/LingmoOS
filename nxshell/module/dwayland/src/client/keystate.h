/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_KEYSTATE_H
#define WAYLAND_KEYSTATE_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_keystate;

namespace KWayland
{
namespace Client
{
class EventQueue;

class KWAYLANDCLIENT_EXPORT Keystate : public QObject
{
    Q_OBJECT
public:
    enum class Key {
        CapsLock = 0,
        NumLock = 1,
        ScrollLock = 2,
    };
    Q_ENUM(Key);
    enum State {
        Unlocked = 0,
        Latched = 1,
        Locked = 2,
    };
    Q_ENUM(State)

    Keystate(QObject *parent);
    ~Keystate() override;

    void setEventQueue(EventQueue *queue);

    void destroy();
    void setup(org_kde_kwin_keystate *keystate);

    void fetchStates();

Q_SIGNALS:
    /**
     * State of the @p key changed to @p state
     */
    void stateChanged(Key key, State state);

    /**
     * The corresponding global for this interface on the Registry got removed.
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
