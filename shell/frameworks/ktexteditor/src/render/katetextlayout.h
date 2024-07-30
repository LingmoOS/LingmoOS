/*
    SPDX-FileCopyrightText: 2002-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2003 Anakim Border <aborder@sources.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_TEXTLAYOUT_H_
#define _KATE_TEXTLAYOUT_H_

#include <QTextLine>

#include "katelinelayout.h"

/**
 * This class represents one visible line of text; with dynamic wrapping,
 * many KateTextLayouts can be needed to represent one actual line of text
 * (ie. one KateLineLayout)
 */
class KateTextLayout
{
    friend class KateLineLayout;
    friend class KateLayoutCache;

public:
    bool isValid() const;
    static KateTextLayout invalid();

    int line() const;
    int virtualLine() const;
    /** Return the index of this visual line inside the document line
        (KateLineLayout).  */
    int viewLine() const;

    const QTextLine &lineLayout() const;
    KateLineLayout *kateLineLayout() const;

    int startCol() const;
    KTextEditor::Cursor start() const;

    /**
     * Return the end column of this text line.
     *
     * \param indicateEOL set to true to return -1 if this layout is the
     *        end of the line, otherwise false to return the end column number
     */
    int endCol(bool indicateEOL = false) const;

    /**
     * Return the end position of this text line.
     *
     * \param indicateEOL set to true to return -1 if this layout is the
     *        end of the line, otherwise false to return the end column number
     */
    KTextEditor::Cursor end(bool indicateEOL = false) const;

    int length() const;
    bool isEmpty() const;

    bool wrap() const;

    bool isDirty() const;
    bool setDirty(bool dirty = true);

    int startX() const;
    int endX() const;
    int width() const;

    int xOffset() const;

    bool isRightToLeft() const;

    bool includesCursor(const KTextEditor::Cursor realCursor) const;

    friend bool operator>(const KateLineLayout &r, const KTextEditor::Cursor c);
    friend bool operator>=(const KateLineLayout &r, const KTextEditor::Cursor c);
    friend bool operator<(const KateLineLayout &r, const KTextEditor::Cursor c);
    friend bool operator<=(const KateLineLayout &r, const KTextEditor::Cursor c);

    void debugOutput() const;

    explicit KateTextLayout(KateLineLayout *line = nullptr, int viewLine = 0);

private:
    KateLineLayout *m_lineLayout;
    QTextLine m_textLayout;

    int m_viewLine;
    mutable int m_startX;
    bool m_invalidDirty = true;
};

#endif
