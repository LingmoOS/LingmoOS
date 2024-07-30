/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_KEYEVENT_H
#define KATEVI_KEYEVENT_H

#include <QKeyEvent>

namespace KateVi
{

/** QEvent wrapper for copying/storing key events.
 *  With Qt6 QEvent itself is no longer copyable/movable and therefore
 *  cannot be held inside containers.
 */
class KeyEvent
{
public:
    QEvent::Type type() const;
    Qt::KeyboardModifiers modifiers() const;
    int key() const;
    QString text() const;

    static KeyEvent fromQKeyEvent(const QKeyEvent &e);

private:
    QEvent::Type m_type = QEvent::None;
    Qt::KeyboardModifiers m_modifiers = Qt::NoModifier;
    int m_key = 0;
    QString m_text;
};

}

#endif // KATEVI_KEYEVENT_H
