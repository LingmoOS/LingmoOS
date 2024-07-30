/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXTBLOCK_H
#define KATE_TEXTBLOCK_H

#include "katetextline.h"

#include <QList>
#include <QSet>
#include <QVarLengthArray>

#include <ktexteditor/cursor.h>
#include <ktexteditor_export.h>

namespace KTextEditor
{
class View;
}

namespace Kate
{
class TextBuffer;
class TextCursor;
class TextRange;

/**
 * Class representing a text block.
 * This is used to build up a Kate::TextBuffer.
 * This class should only be used by TextBuffer/Cursor/Range.
 */
class TextBlock
{
public:
    /**
     * Construct an empty text block.
     * @param buffer parent text buffer
     * @param startLine start line of this block
     */
    TextBlock(TextBuffer *buffer, int startLine);

    /**
     * Destruct the text block
     */
    ~TextBlock();

    /**
     * Start line of this block.
     * @return start line of this block
     */
    int startLine() const
    {
        return m_startLine;
    }

    /**
     * Set start line of this block.
     * @param startLine new start line of this block
     */
    void setStartLine(int startLine);

    /**
     * Retrieve a text line.
     * @param line wanted line number
     * @return text line
     */
    TextLine line(int line) const;

    /**
     * Transfer all non text attributes for the given line from the given text line to the one in the block.
     * @param line line number to set attributes
     * @param textLine line reference to get attributes from
     */
    void setLineMetaData(int line, const TextLine &textLine);

    /**
     * Retrieve length for @p line.
     * @param line wanted line number
     * @return length of line
     */
    int lineLength(int line) const
    {
        Q_ASSERT(line >= startLine() && (line - startLine()) < lines());
        return m_lines[line - startLine()].length();
    }

    /**
     * Append a new line with given text.
     * @param textOfLine text of the line to append
     */
    void appendLine(const QString &textOfLine);

    /**
     * Clear the lines.
     */
    void clearLines();

    /**
     * Number of lines in this block.
     * @return number of lines
     */
    int lines() const
    {
        return static_cast<int>(m_lines.size());
    }

    /**
     * Retrieve text of block.
     * @param text for this block, lines separated by '\n'
     */
    void text(QString &text) const;

    /**
     * Wrap line at given cursor position.
     * @param position line/column as cursor where to wrap
     * @param fixStartLinesStartIndex start index to fix start lines, normally this is this block
     */
    void wrapLine(const KTextEditor::Cursor position, int fixStartLinesStartIndex);

    /**
     * Unwrap given line.
     * @param line line to unwrap
     * @param previousBlock previous block, if any, if we unwrap first line in block, we need to have this
     * @param fixStartLinesStartIndex start index to fix start lines, normally this is this block or the previous one
     */
    void unwrapLine(int line, TextBlock *previousBlock, int fixStartLinesStartIndex);

    /**
     * Insert text at given cursor position.
     * @param position position where to insert text
     * @param text text to insert
     */
    void insertText(const KTextEditor::Cursor position, const QString &text);

    /**
     * Remove text at given range.
     * @param range range of text to remove, must be on one line only.
     * @param removedText will be filled with removed text
     */
    void removeText(KTextEditor::Range range, QString &removedText);

    /**
     * Debug output, print whole block content with line numbers and line length
     * @param blockIndex index of this block in buffer
     */
    void debugPrint(int blockIndex) const;

    /**
     * Split given block. A new block will be created and all lines starting from the given index will
     * be moved to it, together with the cursors belonging to it.
     * @param fromLine line from which to split
     * @return new block containing the lines + cursors removed from this one
     */
    TextBlock *splitBlock(int fromLine);

    /**
     * Merge this block with given one, the given one must be a direct predecessor.
     * @param targetBlock block to merge with
     */
    void mergeBlock(TextBlock *targetBlock);

    /**
     * Delete the block content, delete all lines and delete all cursors not bound to ranges.
     * This is used in destructor of TextBuffer, for fast cleanup. Only stuff remaining afterwards are cursors which are
     * part of a range, TextBuffer will delete them itself...
     */
    void deleteBlockContent();

    /**
     * Clear the block content, delete all lines, move all cursors not bound to range to given block at 0,0.
     * This is used by clear() of TextBuffer.
     * @param targetBlock empty target block for cursors
     */
    void clearBlockContent(TextBlock *targetBlock);

    /**
     * Return all ranges in this block which might intersect the given line.
     * @param line                          line to check intersection
     * @param view                          only return ranges associated with given view
     * @param rangesWithAttributeOnly       ranges with attributes only?
     * @return list of possible candidate ranges
     */
    KTEXTEDITOR_EXPORT QList<TextRange *> rangesForLine(int line, KTextEditor::View *view, bool rangesWithAttributeOnly) const;

    KTEXTEDITOR_NO_EXPORT void rangesForLine(int line, KTextEditor::View *view, bool rangesWithAttributeOnly, QList<TextRange *> &outRanges) const;

    /**
     * Is the given range contained in this block?
     * @param range range to check for
     * @return contained in this blocks mapping?
     */
    bool containsRange(TextRange *range) const
    {
        return m_cachedLineForRanges.find(range) != m_cachedLineForRanges.end() || m_uncachedRanges.contains(range);
    }

    /**
     * Flag all modified text lines as saved on disk.
     */
    void markModifiedLinesAsSaved();

    /**
     * Insert cursor into this block.
     * @param cursor cursor to insert
     */
    void insertCursor(Kate::TextCursor *cursor)
    {
        m_cursors.insert(cursor);
    }

    /**
     * Remove cursor from this block.
     * @param cursor cursor to remove
     */
    void removeCursor(Kate::TextCursor *cursor)
    {
        m_cursors.remove(cursor);
    }

    /**
     * Update a range from this block.
     * Will move the range to right set, either cached for one-line ranges or not.
     * @param range range to update
     */
    void updateRange(TextRange *range);

    /**
     * Remove a range from this block.
     * @param range range to remove
     */
    void removeRange(TextRange *range);

    /**
     * Returns the size of this block i.e.,
     * the count of QChars it has + number of new lines
     */
    int blockSize() const
    {
        return m_blockSize + m_lines.size();
    }

private:
    /**
     * Return all ranges in this block which might intersect the given line and only span one line.
     * For them an internal fast lookup cache is hold.
     * @param line line to check intersection
     * @return set of ranges
     */
    const QVarLengthArray<TextRange *, 6> *cachedRangesForLine(int line) const
    {
        line -= m_startLine;
        if (line >= 0 && (size_t)line < m_cachedRangesForLine.size()) {
            return &m_cachedRangesForLine[line];
        } else {
            return nullptr;
        }
    }

private:
    /**
     * parent text buffer
     */
    TextBuffer *m_buffer;

    /**
     * Lines contained in this buffer.
     * We need no sharing, use STL.
     */
    std::vector<Kate::TextLine> m_lines;

    /**
     * Startline of this block
     */
    int m_startLine;

    /**
     * size of block i.e., number of QChars
     */
    int m_blockSize = 0;

    /**
     * Set of cursors for this block.
     * using QSet is better than unordered_set for perf reasons
     */
    QSet<TextCursor *> m_cursors;

    /**
     * Contains for each line-offset the ranges that were cached into it.
     * These ranges are fully contained by the line.
     */
    std::vector<QVarLengthArray<TextRange *, 6>> m_cachedRangesForLine;

    /**
     * Maps for each cached range the line into which the range was cached.
     */
    QHash<TextRange *, int> m_cachedLineForRanges;

    /**
     * This contains all the ranges that are not cached.
     */
    QVarLengthArray<TextRange *, 1> m_uncachedRanges;
};

}

#endif
