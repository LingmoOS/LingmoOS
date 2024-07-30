/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmodifierkeyinfoprovider_p.h"

KModifierKeyInfoProvider::KModifierKeyInfoProvider()
    : QObject(nullptr)
{
}

KModifierKeyInfoProvider::~KModifierKeyInfoProvider()
{
}

bool KModifierKeyInfoProvider::setKeyLatched(Qt::Key key, bool latched)
{
    Q_UNUSED(key);
    Q_UNUSED(latched);
    return false;
}

bool KModifierKeyInfoProvider::setKeyLocked(Qt::Key key, bool locked)
{
    Q_UNUSED(key);
    Q_UNUSED(locked);
    return false;
}

bool KModifierKeyInfoProvider::isKeyPressed(Qt::Key key) const
{
    auto it = m_modifierStates.constFind(key);
    if (it != m_modifierStates.constEnd()) {
        return *it & Pressed;
    }
    return false;
}

bool KModifierKeyInfoProvider::isKeyLatched(Qt::Key key) const
{
    auto it = m_modifierStates.constFind(key);
    if (it != m_modifierStates.constEnd()) {
        return *it & Latched;
    }
    return false;
}

bool KModifierKeyInfoProvider::isKeyLocked(Qt::Key key) const
{
    auto it = m_modifierStates.constFind(key);
    if (it != m_modifierStates.constEnd()) {
        return *it & Locked;
    }
    return false;
}

bool KModifierKeyInfoProvider::isButtonPressed(Qt::MouseButton button) const
{
    if (m_buttonStates.contains(button)) {
        return m_buttonStates[button];
    }
    return false;
}

bool KModifierKeyInfoProvider::knowsKey(Qt::Key key) const
{
    return m_modifierStates.contains(key);
}

const QList<Qt::Key> KModifierKeyInfoProvider::knownKeys() const
{
    return m_modifierStates.keys();
}

void KModifierKeyInfoProvider::stateUpdated(Qt::Key key, KModifierKeyInfoProvider::ModifierStates newState)
{
    auto &state = m_modifierStates[key];
    if (newState != state) {
        const auto difference = (newState ^ state);
        state = newState;
        if (difference & Pressed) {
            Q_EMIT keyPressed(key, newState & Pressed);
        }
        if (difference & Latched) {
            Q_EMIT keyLatched(key, newState & Latched);
        }
        if (difference & Locked) {
            Q_EMIT keyLocked(key, newState & Locked);
        }
    }
}

#include "moc_kmodifierkeyinfoprovider_p.cpp"
