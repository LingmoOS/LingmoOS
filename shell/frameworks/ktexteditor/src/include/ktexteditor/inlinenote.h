/*
    SPDX-FileCopyrightText: 2018 Sven Brauch <mail@svenbrauch.de>
    SPDX-FileCopyrightText: 2018 Michal Srb <michalsrb@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_INLINENOTE_H
#define KTEXTEDITOR_INLINENOTE_H

#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>

class QFont;
class KateInlineNoteData;
namespace KTextEditor
{
class InlineNoteProvider;
}

namespace KTextEditor
{
/**
 * @class InlineNote inlinenote.h <KTextEditor/InlineNote>
 *
 * Describes an inline note.
 *
 * This class contains all the information required to deal with a particular
 * inline note. It is instantiated and populated with information internally by
 * KTextEditor based on the list of notes returned by InlineNoteProvider::inlineNotes(),
 * and then passed back to the user of the API.
 *
 * @note Users of the InlineNoteInterface API should never create a InlineNote
 *       themselves. Maybe it helps to think of a InlineNote as if it were a
 *       QModelIndex. Only the internal KTextEditor implementation creates them.
 *
 * @since 5.50
 */
class KTEXTEDITOR_EXPORT InlineNote
{
public:
    /**
     * Constructs an inline note. User code never calls this constructor,
     * since notes are created internally only from the columns returned by
     * InlineNoteProvider::inlineNotes(), and then passed around as handles
     * grouping useful information.
     */
    InlineNote(const KateInlineNoteData &data);

    /**
     * Returns the width of this note in pixels.
     */
    qreal width() const;

    /**
     * The provider which created this note
     */
    InlineNoteProvider *provider() const;

    /**
     * The View this note is shown in.
     */
    const KTextEditor::View *view() const;

    /**
     * The cursor position of this note.
     */
    KTextEditor::Cursor position() const;

    /**
     * The index of this note, i.e. its index in the vector returned by
     * the provider for a given line
     */
    int index() const;

    /**
     * Returns whether the mouse cursor is currently over this note.
     * @note This flag is useful when in InlineNoteProvider::paintInlineNote().
     */
    bool underMouse() const;

    /**
     * The font of the text surrounding this note.
     * This can be used to obtain the QFontMetrics or similar font information.
     */
    QFont font() const;

    /**
     * The height of the line containing this note
     */
    int lineHeight() const;

private:
    // Internal implementation data structure.
    const KateInlineNoteData &d;
};

}

#endif
