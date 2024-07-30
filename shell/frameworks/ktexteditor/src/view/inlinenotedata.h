/*
    SPDX-FileCopyrightText: 2018 Sven Brauch <mail@svenbrauch.de>
    SPDX-FileCopyrightText: 2018 Michal Srb <michalsrb@gmail.com>
    SPDX-FileCopyrightText: 2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_INLINENOTE_DATA_H
#define KATE_INLINENOTE_DATA_H

#include <QFont>
#include <ktexteditor/cursor.h>

namespace KTextEditor
{
class InlineNoteProvider;
class View;
}

/**
 * Internal data container for KTextEditor::InlineNote interface.
 */
class KateInlineNoteData
{
public:
    KateInlineNoteData() = default;
    KateInlineNoteData(KTextEditor::InlineNoteProvider *provider,
                       const KTextEditor::View *view,
                       const KTextEditor::Cursor position,
                       int index,
                       bool underMouse,
                       const QFont &font,
                       int lineHeight);

    KTextEditor::InlineNoteProvider *m_provider = nullptr;
    const KTextEditor::View *m_view = nullptr;
    KTextEditor::Cursor m_position = KTextEditor::Cursor::invalid();
    int m_index = -1;
    bool m_underMouse = false;
    QFont m_font;
    int m_lineHeight = -1;
};

#endif
