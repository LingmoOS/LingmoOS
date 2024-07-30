/*
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2002-2004 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_BUFFER_H
#define KATE_BUFFER_H

#include "katehighlight.h"
#include "katetextbuffer.h"

#include <ktexteditor_export.h>

#include <QObject>

class KateLineInfo;
namespace KTextEditor
{
class DocumentPrivate;
}

/**
 * The KateBuffer class maintains a collections of lines.
 *
 * @author Waldo Bastian <bastian@kde.org>
 * @author Christoph Cullmann <cullmann@kde.org>
 */
class KTEXTEDITOR_EXPORT KateBuffer final : public Kate::TextBuffer
{
    Q_OBJECT

public:
    /**
     * Create an empty buffer.
     * @param doc parent document
     */
    explicit KateBuffer(KTextEditor::DocumentPrivate *doc);

    /**
     * Goodbye buffer
     */
    ~KateBuffer() override;

public:
    /**
     * start some editing action
     */
    void editStart();

    /**
     * finish some editing action
     */
    void editEnd();

    /**
     * Update highlighting of the lines in
     * last edit transaction
     */
    void updateHighlighting();

    /**
     * were there changes in the current running
     * editing session?
     * @return changes done?
     */
    inline bool editChanged() const
    {
        return editingChangedBuffer();
    }

    /**
     * dirty lines start
     * @return start line
     */
    inline int editTagStart() const
    {
        return editingMinimalLineChanged();
    }

    /**
     * dirty lines end
     * @return end line
     */
    inline int editTagEnd() const
    {
        return editingMaximalLineChanged();
    }

    /**
     * line inserted/removed?
     * @return line inserted/removed?
     */
    inline bool editTagFrom() const
    {
        return editingChangedNumberOfLines() != 0;
    }

public:
    /**
     * Clear the buffer.
     */
    void clear() override;

    /**
     * Open a file, use the given filename
     * @param m_file filename to open
     * @param enforceTextCodec enforce to use only the set text codec
     * @return success
     */
    bool openFile(const QString &m_file, bool enforceTextCodec);

    /**
     * Did encoding errors occur on load?
     * @return encoding errors occurred on load?
     */
    bool brokenEncoding() const
    {
        return m_brokenEncoding;
    }

    /**
     * Too long lines wrapped on load?
     * @return too long lines wrapped on load?
     */
    bool tooLongLinesWrapped() const
    {
        return m_tooLongLinesWrapped;
    }

    int longestLineLoaded() const
    {
        return m_longestLineLoaded;
    }

    /**
     * Can the current codec handle all chars
     * @return chars can be encoded
     */
    bool canEncode();

    /**
     * Save the buffer to a file, use the given filename + codec + end of line chars (internal use of qtextstream)
     * @param m_file filename to save to
     * @return success
     */
    bool saveFile(const QString &m_file);

public:
    /**
     * Return line @p lineno.
     * Highlighting of returned line might be out-dated, which may be sufficient
     * for pure text manipulation functions, like search/replace.
     * If you require highlighting to be up to date, call @ref ensureHighlighted
     * prior to this method.
     */
    inline Kate::TextLine plainLine(int lineno)
    {
        if (lineno < 0 || lineno >= lines()) {
            return Kate::TextLine();
        }

        return line(lineno);
    }

    int lineLength(int lineno) const
    {
        if (lineno < 0 || lineno >= lines()) {
            return -1;
        }

        return Kate::TextBuffer::lineLength(lineno);
    }

    /**
     * Update highlighting of given line @p line, if needed.
     * If @p line is already highlighted, this function does nothing.
     * If @p line is not highlighted, all lines up to line + lookAhead
     * are highlighted.
     * @param lookAhead also highlight these following lines
     */
    void ensureHighlighted(int line, int lookAhead = 64);

    /**
     * Unwrap given line.
     * @param line line to unwrap
     */
    void unwrapLine(int line) override;

    /**
     * Wrap line at given cursor position.
     * @param position line/column as cursor where to wrap
     */
    void wrapLine(const KTextEditor::Cursor position) override;

public:
    inline int tabWidth() const
    {
        return m_tabWidth;
    }

public:
    void setTabWidth(int w);

    /**
     * Use @p highlight for highlighting
     *
     * @p highlight may be 0 in which case highlighting
     * will be disabled.
     */
    void setHighlight(int hlMode);

    KateHighlighting *highlight()
    {
        return m_highlight;
    }

    /**
     * Invalidate highlighting of whole buffer.
     */
    void invalidateHighlighting();

    /**
     * Compute folding vector for the given line, will internally do a re-highlighting.
     * @param line line to get folding vector for
     */
    KateHighlighting::Foldings computeFoldings(int line);

    /**
     * For a given line, compute if folding starts here.
     * @param startLine start line
     * @return does folding start here and is it indentation based?
     */
    std::pair<bool, bool> isFoldingStartingOnLine(int startLine);

    /**
     * For a given line, compute the folding range that starts there
     * to be used to fold e.g. from the icon border
     * @param startLine start line
     * @return folding range starting at the given line or invalid range when
     *         there is no folding start or @p startLine is not valid
     */
    KTextEditor::Range computeFoldingRangeForStartLine(int startLine);

private:
    /**
     * Highlight information needs to be updated.
     *
     * @param from first line in range
     * @param to last line in range
     * @param invalidate should the rehighlighted lines be tagged?
     */
    KTEXTEDITOR_NO_EXPORT
    void doHighlight(int from, int to, bool invalidate);

Q_SIGNALS:
    /**
     * Emitted when the highlighting of a certain range has
     * changed.
     */
    void tagLines(KTextEditor::LineRange lineRange);
    void respellCheckBlock(int start, int end);

private:
    /**
     * document we belong to
     */
    KTextEditor::DocumentPrivate *const m_doc;

    /**
     * file loaded with encoding problems?
     */
    bool m_brokenEncoding;

    /**
     * too long lines wrapped on load?
     */
    bool m_tooLongLinesWrapped;

    /**
     * length of the longest line loaded
     */
    int m_longestLineLoaded;

    /**
     * current highlighting mode or 0
     */
    KateHighlighting *m_highlight;

    // for the scrapty indent sensitive langs
    int m_tabWidth;

    /**
     * last line with valid highlighting
     */
    int m_lineHighlighted;
};

#endif
