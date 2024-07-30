/*
    SPDX-FileCopyrightText: 2002-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2003 Anakim Border <aborder@sources.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_LINELAYOUT_H_
#define _KATE_LINELAYOUT_H_

#include <QExplicitlySharedDataPointer>
#include <QSharedData>

#include <optional>

#include "katetextline.h"

#include <ktexteditor/cursor.h>

class QTextLayout;
namespace KTextEditor
{
class DocumentPrivate;
}
class KateTextLayout;
class KateRenderer;

class KateLineLayout
{
public:
    explicit KateLineLayout(KateRenderer &renderer);

    void debugOutput() const;

    void clear();
    bool isValid() const;
    bool isOutsideDocument() const;

    bool isRightToLeft() const;

    bool includesCursor(const KTextEditor::Cursor realCursor) const;

    friend bool operator>(const KateLineLayout &r, const KTextEditor::Cursor c);
    friend bool operator>=(const KateLineLayout &r, const KTextEditor::Cursor c);
    friend bool operator<(const KateLineLayout &r, const KTextEditor::Cursor c);
    friend bool operator<=(const KateLineLayout &r, const KTextEditor::Cursor c);

    const Kate::TextLine &textLine(bool forceReload = false) const;
    int length() const;

    int line() const;
    /**
     * Only pass virtualLine if you know it (and thus we shouldn't try to look it up)
     */
    void setLine(int line, int virtualLine = -1);
    KTextEditor::Cursor start() const;

    int virtualLine() const;
    void setVirtualLine(int virtualLine);

    bool isDirty(int viewLine) const;
    bool setDirty(int viewLine, bool dirty = true);

    int width() const;
    int widthOfLastLine();

    int viewLineCount() const;
    KateTextLayout viewLine(int viewLine);
    int viewLineForColumn(int column) const;

    bool startsInvisibleBlock() const;

    QTextLayout *layout() const;
    void setLayout(QTextLayout *layout);
    void invalidateLayout();

    bool layoutDirty = true;
    bool usePlainTextLine = false;

    // This variable is used as follows:
    // non-dynamic-wrapping mode: unused
    // dynamic wrapping mode:
    //   first viewLine of a line: the X position of the first non-whitespace char
    //   subsequent viewLines: the X offset from the left of the display.
    //
    // this is used to provide a dynamic-wrapping-retains-indent feature.
    int shiftX = 0;

private:
    // Disable copy
    KateLineLayout(const KateLineLayout &copy);

    KateRenderer &m_renderer;
    mutable std::optional<Kate::TextLine> m_textLine;
    int m_line;
    int m_virtualLine;

    std::unique_ptr<QTextLayout> m_layout;
    QList<bool> m_dirtyList;
};

#endif
