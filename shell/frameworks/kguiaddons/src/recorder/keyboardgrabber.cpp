/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#include "keyboardgrabber_p.h"

#include <QWindow>

KeyboardGrabber::KeyboardGrabber(QWindow *window)
    : ShortcutInhibition()
    , m_grabbedWindow(window)
    , m_grabbingKeyboard(false)
{
}

KeyboardGrabber::~KeyboardGrabber()
{
    disableInhibition();
}

void KeyboardGrabber::enableInhibition()
{
    if (m_grabbingKeyboard || !m_grabbedWindow) {
        return;
    }
    m_grabbingKeyboard = m_grabbedWindow->setKeyboardGrabEnabled(true);
}

void KeyboardGrabber::disableInhibition()
{
    if (!m_grabbingKeyboard) {
        return;
    }
    m_grabbingKeyboard = !(m_grabbedWindow->setKeyboardGrabEnabled(false));
}

bool KeyboardGrabber::shortcutsAreInhibited() const
{
    return m_grabbingKeyboard;
}
