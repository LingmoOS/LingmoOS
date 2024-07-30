/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jumps.h"

using namespace KateVi;

void Jumps::add(const KTextEditor::Cursor cursor)
{
    for (auto iterator = m_jumps.begin(); iterator != m_jumps.end(); iterator++) {
        if ((*iterator).line() == cursor.line()) {
            m_jumps.erase(iterator);
            break;
        }
    }

    m_jumps.push_back(cursor);
    m_current = m_jumps.end();
}

KTextEditor::Cursor Jumps::next(const KTextEditor::Cursor cursor)
{
    if (m_current == m_jumps.end()) {
        return cursor;
    }

    KTextEditor::Cursor jump;
    if (m_current + 1 != m_jumps.end()) {
        jump = *(++m_current);
    } else {
        jump = *(m_current);
    }

    return jump;
}

KTextEditor::Cursor Jumps::prev(const KTextEditor::Cursor cursor)
{
    if (m_current == m_jumps.end()) {
        add(cursor);
        m_current--;
    }

    if (m_current != m_jumps.begin()) {
        m_current--;
        return *m_current;
    }

    return cursor;
}

void Jumps::readSessionConfig(const KConfigGroup &config)
{
    // Format: jump1.line, jump1.column, jump2.line, jump2.column, jump3.line, ...
    m_jumps.clear();
    QStringList jumps = config.readEntry("JumpList", QStringList());

    for (int i = 0; i + 1 < jumps.size(); i += 2) {
        KTextEditor::Cursor jump = {jumps.at(i).toInt(), jumps.at(i + 1).toInt()};
        m_jumps.push_back(jump);
    }

    m_current = m_jumps.end();
}

void Jumps::writeSessionConfig(KConfigGroup &config) const
{
    if (m_jumps.isEmpty()) {
        return;
    }
    // Format: jump1.line, jump1.column, jump2.line, jump2.column, jump3.line, ...
    QStringList l;
    for (const auto &jump : m_jumps) {
        l << QString::number(jump.line()) << QString::number(jump.column());
    }
    config.writeEntry("JumpList", l);
}
