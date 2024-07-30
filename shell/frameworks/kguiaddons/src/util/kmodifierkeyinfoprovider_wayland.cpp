/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmodifierkeyinfoprovider_wayland.h"
#include <QDebug>
#include <QGuiApplication>

#include <QWaylandClientExtensionTemplate>
#include <wayland-client-core.h>

#include "qwayland-keystate.h"

class KeyState : public QWaylandClientExtensionTemplate<KeyState>, public QtWayland::org_kde_kwin_keystate
{
    Q_OBJECT
public:
    KeyState()
        : QWaylandClientExtensionTemplate<KeyState>(4)
    {
    }

    ~KeyState()
    {
        if (isInitialized() && qGuiApp) {
            if (QtWayland::org_kde_kwin_keystate::version() >= ORG_KDE_KWIN_KEYSTATE_DESTROY_SINCE_VERSION) {
                destroy();
            } else {
                wl_proxy_destroy(reinterpret_cast<struct wl_proxy *>(object()));
            }
        }
    }

    void org_kde_kwin_keystate_stateChanged(uint32_t key, uint32_t state) override
    {
        Q_EMIT stateChanged(toKey(static_cast<KeyState::key>(key)), toState(static_cast<KeyState::state>(state)));
    }

    KModifierKeyInfoProvider::ModifierState toState(KeyState::state state)
    {
        switch (state) {
        case KeyState::state::state_unlocked:
            return KModifierKeyInfoProvider::Nothing;
        case KeyState::state::state_locked:
            return KModifierKeyInfoProvider::Locked;
        case KeyState::state::state_latched:
            return KModifierKeyInfoProvider::Latched;
        }
        Q_UNREACHABLE();
        return KModifierKeyInfoProvider::Nothing;
    }

    Qt::Key toKey(KeyState::key key)
    {
        switch (key) {
        case KeyState::key::key_capslock:
            return Qt::Key_CapsLock;
        case KeyState::key::key_numlock:
            return Qt::Key_NumLock;
        case KeyState::key::key_scrolllock:
            return Qt::Key_ScrollLock;
        }
        Q_UNREACHABLE();
        return {};
    }

    Q_SIGNAL void stateChanged(Qt::Key key, KModifierKeyInfoProvider::ModifierState state);
};

KModifierKeyInfoProviderWayland::KModifierKeyInfoProviderWayland()
{
    m_keystate = new KeyState;

    QObject::connect(m_keystate, &KeyState::activeChanged, this, [this]() {
        if (m_keystate->isActive()) {
            m_keystate->fetchStates();
        }
    });

    connect(m_keystate, &KeyState::stateChanged, this, &KModifierKeyInfoProviderWayland::stateUpdated);

    stateUpdated(Qt::Key_CapsLock, KModifierKeyInfoProvider::Nothing);
    stateUpdated(Qt::Key_NumLock, KModifierKeyInfoProvider::Nothing);
    stateUpdated(Qt::Key_ScrollLock, KModifierKeyInfoProvider::Nothing);
}

KModifierKeyInfoProviderWayland::~KModifierKeyInfoProviderWayland()
{
    delete m_keystate;
}

bool KModifierKeyInfoProviderWayland::setKeyLatched(Qt::Key /*key*/, bool /*latched*/)
{
    return false;
}

bool KModifierKeyInfoProviderWayland::setKeyLocked(Qt::Key /*key*/, bool /*locked*/)
{
    return false;
}

#include "kmodifierkeyinfoprovider_wayland.moc"
#include "moc_kmodifierkeyinfoprovider_wayland.cpp"
