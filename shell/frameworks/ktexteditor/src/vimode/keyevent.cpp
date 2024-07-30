/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "keyevent.h"

using namespace KateVi;

QEvent::Type KeyEvent::type() const
{
    return m_type;
}

Qt::KeyboardModifiers KeyEvent::modifiers() const
{
    return m_modifiers;
}

int KeyEvent::key() const
{
    return m_key;
}

QString KeyEvent::text() const
{
    return m_text;
}

KeyEvent KeyEvent::fromQKeyEvent(const QKeyEvent &e)
{
    KeyEvent keyEvent;
    keyEvent.m_type = e.type();
    keyEvent.m_modifiers = e.modifiers();
    keyEvent.m_key = e.key();
    keyEvent.m_text = e.text();
    return keyEvent;
}
