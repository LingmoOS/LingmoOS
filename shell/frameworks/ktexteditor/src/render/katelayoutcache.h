/*
    SPDX-FileCopyrightText: 2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATELAYOUTCACHE_H
#define KATELAYOUTCACHE_H

#include <QPair>

#include <ktexteditor/range.h>

#include "katetextlayout.h"

class KateRenderer;

class KateLineLayoutMap
{
public:
    void clear();

    void insert(int realLine, std::unique_ptr<KateLineLayout> lineLayoutPtr);

    void relayoutLines(int startRealLine, int endRealLine);

    void slotEditDone(int fromLine, int toLine, int shiftAmount, std::vector<KateTextLayout> &textLayouts);

    KateLineLayout *find(int i);

    typedef std::pair<int, std::unique_ptr<KateLineLayout>> LineLayoutPair;

private:
    typedef std::vector<LineLayoutPair> LineLayoutMap;
    LineLayoutMap m_lineLayouts;
};

/**
 * This class handles Kate's caching of layouting information (in KateLineLayout
 * and KateTextLayout).  This information is used primarily by both the view and
 * the renderer.
 *
 * We outsource the hardcore layouting logic to the renderer, but other than
 * that, this class handles all manipulation of the layout objects.
 *
 * This is separate from the renderer 1) for clarity 2) so you can have separate
 * caches for separate views of the same document, even for view and printer
 * (if the renderer is made to support rendering onto different targets).
 *
 * @author Hamish Rodda \<rodda@kde.org\>
 */

class KateLayoutCache : public QObject
{
public:
    explicit KateLayoutCache(KateRenderer *renderer, QObject *parent);

    void clear();

    int viewWidth() const;
    void setViewWidth(int width);

    bool wrap() const;
    void setWrap(bool wrap);

    bool acceptDirtyLayouts() const;
    void setAcceptDirtyLayouts(bool accept);

    // BEGIN generic methods to get/set layouts
    /**
     * Returns the KateLineLayout for the specified line.
     *
     * If one does not exist, it will be created and laid out.
     * Layouts which are not directly part of the view will be kept until the
     * cache is full or until they are invalidated by other means (eg. the text
     * changes).
     *
     * This function shall never return null
     *
     * \param realLine real line number of the layout to retrieve.
     * \param virtualLine virtual line number. only needed if you think it may have changed
     *                    (ie. basically internal to KateLayoutCache)
     */
    KateLineLayout *line(int realLine, int virtualLine = -1);

    /// Returns the layout describing the text line which is occupied by \p realCursor.
    KateTextLayout textLayout(const KTextEditor::Cursor realCursor);

    /// Returns the layout of the specified realLine + viewLine.
    /// if viewLine is -1, return the last.
    KateTextLayout textLayout(uint realLine, int viewLine);
    // END

    // BEGIN methods to do with the caching of lines visible within a view
    /// Returns the layout of the corresponding line in the view
    KateTextLayout &viewLine(int viewLine);

    /**
     * Find the view line of the cursor, relative to the display (0 = top line of view, 1 = second line, etc.)
     *
     * If @p limitToVisible is true, the function can return -2 for lines below the view. The idea is to get extra
     * information about where the line lies when its out of the view so the clients doesn't have to make second
     * call of this function with limitToVisible = false and potentionaly rerendering the whole document.
     *
     * \param virtualCursor cursor position
     * \param limitToVisible if true, limit the search to only visible lines
     *
     * @return line number relative to the display. If @p limitToVisible is true,
     * then valid values are only positive, negative values are invalid cursors for -1 and -2 for cursor is
     * below the view.
     */
    int displayViewLine(const KTextEditor::Cursor virtualCursor, bool limitToVisible = false);

    int viewCacheLineCount() const;
    KTextEditor::Cursor viewCacheStart() const;
    KTextEditor::Cursor viewCacheEnd() const;
    void updateViewCache(const KTextEditor::Cursor startPos, int newViewLineCount = -1, int viewLinesScrolled = 0);

    void relayoutLines(int startRealLine, int endRealLine);

    // find the index of the last view line for a specific line
    int lastViewLine(int realLine);
    // find the view line of cursor c (0 = same line, 1 = down one, etc.)
    int viewLine(const KTextEditor::Cursor realCursor);
    int viewLineCount(int realLine);

    void viewCacheDebugOutput() const;
    // END

private:
    void wrapLine(KTextEditor::Document *, const KTextEditor::Cursor position);
    void unwrapLine(KTextEditor::Document *, int line);
    void insertText(KTextEditor::Document *, const KTextEditor::Cursor position, const QString &text);
    void removeText(KTextEditor::Document *, KTextEditor::Range range, const QString &);

private:
    KateRenderer *m_renderer;

    /**
     * The master cache of all line layouts.
     *
     * Layouts which are not within the current view cache and whose
     * refcount == 1 are only known to the cache and can be safely deleted.
     */
    KateLineLayoutMap m_lineLayouts;

    // Convenience vector for quick direct access to the specific text layout
    KTextEditor::Cursor m_startPos = KTextEditor::Cursor::invalid();

    std::vector<KateTextLayout> m_textLayouts;

    int m_viewWidth = 0;
    bool m_wrap = false;
    bool m_acceptDirtyLayouts = false;
};

#endif
