/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "note.h"

Note::Note(const QString &id)
    : QObject()
    , m_id(id)
{
}

QString Note::id() const
{
    return m_id;
}

void Note::setNoteText(const QString &text)
{
    if (text == m_noteText) {
        return;
    }
    m_noteText = text;
    Q_EMIT noteTextChanged();
}

QString Note::noteText() const
{
    return m_noteText;
}
