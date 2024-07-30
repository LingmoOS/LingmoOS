/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KATE_TEXTBUFFER_H
#define KATE_TEXTBUFFER_H

#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

#include "katetextblock.h"
#include "katetexthistory.h"
#include <ktexteditor_export.h>

// encoding prober
#include <KEncodingProber>

namespace KTextEditor
{
class DocumentPrivate;
}

class KCompressionDevice;

namespace Kate
{
class TextRange;
class TextCursor;
class TextBlock;

constexpr int BufferBlockSize = 64;

/**
 * Class representing a text buffer.
 * The interface is line based, internally the text will be stored in blocks of text lines.
 */
class KTEXTEDITOR_EXPORT TextBuffer : public QObject
{
    friend class TextCursor;
    friend class TextRange;
    friend class TextBlock;

    Q_OBJECT

public:
    /**
     * End of line mode
     */
    enum EndOfLineMode { eolUnknown = -1, eolUnix = 0, eolDos = 1, eolMac = 2 };

    /**
     * Construct an empty text buffer.
     * Empty means one empty line in one block.
     * @param parent parent qobject
     */
    explicit TextBuffer(KTextEditor::DocumentPrivate *parent, bool alwaysUseKAuth = false);

    /**
     * Destruct the text buffer
     * Virtual, we allow inheritance
     */
    ~TextBuffer() override;

    /**
     * Clears the buffer, reverts to initial empty state.
     * Empty means one empty line in one block.
     * Virtual, can be overwritten.
     */
    virtual void clear();

    /**
     * Set encoding prober type for this buffer to use for load.
     * @param proberType prober type to use for encoding
     */
    void setEncodingProberType(KEncodingProber::ProberType proberType)
    {
        m_encodingProberType = proberType;
    }

    /**
     * Get encoding prober type for this buffer
     * @return currently in use prober type of this buffer
     */
    KEncodingProber::ProberType encodingProberType() const
    {
        return m_encodingProberType;
    }

    /**
     * Set fallback codec for this buffer to use for load.
     * @param codec fallback to use for encoding
     */
    void setFallbackTextCodec(const QString &codec)
    {
        m_fallbackTextCodec = codec;
    }

    /**
     * Get fallback codec for this buffer
     * @return currently in use fallback codec of this buffer
     */
    QString fallbackTextCodec() const
    {
        return m_fallbackTextCodec;
    }

    /**
     * Set codec for this buffer to use for load/save.
     * Loading might overwrite this, if it encounters problems and finds a better codec.
     * Might change BOM setting.
     * @param codec to use for encoding
     */
    void setTextCodec(const QString &codec);

    /**
     * Get codec for this buffer
     * @return currently in use codec of this buffer
     */
    QString textCodec() const
    {
        return m_textCodec;
    }

    /**
     * Generate byte order mark on save.
     * Loading might overwrite this setting, if there is a BOM found inside the file.
     * @param generateByteOrderMark should BOM be generated?
     */
    void setGenerateByteOrderMark(bool generateByteOrderMark)
    {
        m_generateByteOrderMark = generateByteOrderMark;
    }

    /**
     * Generate byte order mark on save?
     * @return should BOM be generated?
     */
    bool generateByteOrderMark() const
    {
        return m_generateByteOrderMark;
    }

    /**
     * Set end of line mode for this buffer, not allowed to be set to unknown.
     * Loading might overwrite this setting, if there is a eol found inside the file.
     * @param endOfLineMode new eol mode
     */
    void setEndOfLineMode(EndOfLineMode endOfLineMode)
    {
        Q_ASSERT(endOfLineMode != eolUnknown);
        m_endOfLineMode = endOfLineMode;
    }

    /**
     * Get end of line mode
     * @return end of line mode
     */
    EndOfLineMode endOfLineMode() const
    {
        return m_endOfLineMode;
    }

    /**
     * Set line length limit
     * @param lineLengthLimit new line length limit
     */
    void setLineLengthLimit(int lineLengthLimit)
    {
        m_lineLengthLimit = lineLengthLimit;
    }

    /**
     * Load the given file. This will first clear the buffer and then load the file.
     * Even on error during loading the buffer will still be cleared.
     * Before calling this, setTextCodec must have been used to set codec!
     * @param filename file to open
     * @param encodingErrors were there problems occurred while decoding the file?
     * @param tooLongLinesWrapped were too long lines found and wrapped?
     * @param longestLineLoaded the longest line in the file (before wrapping)
     * @param enforceTextCodec enforce to use only the set text codec
     * @return success, the file got loaded, perhaps with encoding errors
     * Virtual, can be overwritten.
     */
    virtual bool load(const QString &filename, bool &encodingErrors, bool &tooLongLinesWrapped, int &longestLineLoaded, bool enforceTextCodec);

    /**
     * Save the current buffer content to the given file.
     * Before calling this, setTextCodec and setFallbackTextCodec must have been used to set codec!
     * @param filename file to save
     * @return success
     * Virtual, can be overwritten.
     */
    virtual bool save(const QString &filename);

    /**
     * Lines currently stored in this buffer.
     * This is never 0, even clear will let one empty line remain.
     */
    int lines() const
    {
        Q_ASSERT(m_lines > 0);
        return m_lines;
    }

    /**
     * Revision of this buffer. Is set to 0 on construction, clear() (load will trigger clear()).
     * Is incremented on each change to the buffer.
     * @return current revision
     */
    qint64 revision() const
    {
        return m_revision;
    }

    /**
     * Retrieve a text line.
     * @param line wanted line number
     * @return text line
     */
    TextLine line(int line) const;

    /**
     * Transfer all non text attributes for the given line from the given text line to the one in the buffer.
     * @param line line number to set attributes
     * @param textLine line reference to get attributes from
     */
    void setLineMetaData(int line, const TextLine &textLine);

    /**
     * Retrieve length for @p line
     * @param line wanted line number
     * @return length of the line
     */
    int lineLength(int line) const
    {
        // get block, this will assert on invalid line
        int blockIndex = blockForLine(line);

        // get line length
        return m_blocks.at(blockIndex)->lineLength(line);
    }

    /**
     * Retrieve offset in text for the given cursor position
     */
    int cursorToOffset(KTextEditor::Cursor c) const;

    /**
     * Retrieve cursor in text for the given offset
     */
    KTextEditor::Cursor offsetToCursor(int offset) const;

    /**
     * Retrieve text of complete buffer.
     * @return text for this buffer, lines separated by '\n'
     */
    QString text() const;

    /**
     * Start an editing transaction, the wrapLine/unwrapLine/insertText and removeText functions
     * are only allowed to be called inside a editing transaction.
     * Editing transactions can stack. The number of startEdit and endEdit calls must match.
     * @return returns true, if no transaction was already running
     * Virtual, can be overwritten.
     */
    virtual bool startEditing();

    /**
     * Finish an editing transaction. Only allowed to be called if editing transaction is started.
     * @return returns true, if this finished last running transaction
     * Virtual, can be overwritten.
     */
    virtual bool finishEditing();

    /**
     * Query the number of editing transactions running atm.
     * @return number of running transactions
     */
    int editingTransactions() const
    {
        return m_editingTransactions;
    }

    /**
     * Query the revision of this buffer before the ongoing editing transactions.
     * @return revision of buffer before current editing transaction altered it
     */
    qint64 editingLastRevision() const
    {
        return m_editingLastRevision;
    }

    /**
     * Query the number of lines of this buffer before the ongoing editing transactions.
     * @return number of lines of buffer before current editing transaction altered it
     */
    int editingLastLines() const
    {
        return m_editingLastLines;
    }

    /**
     * Query information from the last editing transaction: was the content of the buffer changed?
     * This is checked by comparing the editingLastRevision() with the current revision().
     * @return content of buffer was changed in last transaction?
     */
    bool editingChangedBuffer() const
    {
        return editingLastRevision() != revision();
    }

    /**
     * Query information from the last editing transaction: was the number of lines of the buffer changed?
     * This is checked by comparing the editingLastLines() with the current lines().
     * @return content of buffer was changed in last transaction?
     */
    bool editingChangedNumberOfLines() const
    {
        return editingLastLines() != lines();
    }

    /**
     * Get minimal line number changed by last editing transaction
     * @return maximal line number changed by last editing transaction, or -1, if none changed
     */
    int editingMinimalLineChanged() const
    {
        return m_editingMinimalLineChanged;
    }

    /**
     * Get maximal line number changed by last editing transaction
     * @return maximal line number changed by last editing transaction, or -1, if none changed
     */
    int editingMaximalLineChanged() const
    {
        return m_editingMaximalLineChanged;
    }

    /**
     * Wrap line at given cursor position.
     * @param position line/column as cursor where to wrap
     * Virtual, can be overwritten.
     */
    virtual void wrapLine(const KTextEditor::Cursor position);

    /**
     * Unwrap given line.
     * @param line line to unwrap
     * Virtual, can be overwritten.
     */
    virtual void unwrapLine(int line);

    /**
     * Insert text at given cursor position. Does nothing if text is empty, beside some consistency checks.
     * @param position position where to insert text
     * @param text text to insert
     * Virtual, can be overwritten.
     */
    virtual void insertText(const KTextEditor::Cursor position, const QString &text);

    /**
     * Remove text at given range. Does nothing if range is empty, beside some consistency checks.
     * @param range range of text to remove, must be on one line only.
     * Virtual, can be overwritten.
     */
    virtual void removeText(KTextEditor::Range range);

    /**
     * TextHistory of this buffer
     * @return text history for this buffer
     */
    TextHistory &history()
    {
        return m_history;
    }

Q_SIGNALS:
    /**
     * Buffer got cleared. This is emitted when constructor or load have called clear() internally,
     * or when the user of the buffer has called clear() itself.
     */
    void cleared();

    /**
     * Buffer loaded successfully a file
     * @param filename file which was loaded
     * @param encodingErrors were there problems occurred while decoding the file?
     */
    void loaded(const QString &filename, bool encodingErrors);

    /**
     * Buffer saved successfully a file
     * @param filename file which was saved
     */
    void saved(const QString &filename);

private:
    /**
     * Save result which indicates an abstract reason why the operation has
     * failed
     */
    enum class SaveResult { Failed = 0, MissingPermissions, Success };

    /**
     * Find block containing given line.
     * @param line we want to find block for this line
     * @return index of found block
     */
    int blockForLine(int line) const;
    // exported for movingrange_test

    /**
     * Fix start lines of all blocks after the given one
     * @param startBlock index of block from which we start to fix
     */
    KTEXTEDITOR_NO_EXPORT
    void fixStartLines(int startBlock);

    /**
     * Balance the given block. Look if it is too small or too large.
     * @param index block to balance
     */
    KTEXTEDITOR_NO_EXPORT
    void balanceBlock(int index);

    /**
     * A range changed, notify the views, in case of attributes or feedback.
     * @param view which view is affected? nullptr for all views
     * @param lineRange line range that the change spans
     * @param needsRepaint do we need to trigger repaints? e.g. if ranges with attributes change
     */
    KTEXTEDITOR_NO_EXPORT
    void notifyAboutRangeChange(KTextEditor::View *view, KTextEditor::LineRange lineRange, bool needsRepaint);

    /**
     * Mark all modified lines as lines saved on disk (modified line system).
     */
    KTEXTEDITOR_NO_EXPORT
    void markModifiedLinesAsSaved();

    /**
     * Save the current buffer content to the given already opened device
     *
     * @param filename path name for display/debugging purposes
     * @param saveFile open device to write the buffer to
     */
    KTEXTEDITOR_NO_EXPORT
    bool saveBuffer(const QString &filename, KCompressionDevice &saveFile);

    /**
     * Attempt to save the buffer content in the given filename location using
     * current privileges.
     */
    KTEXTEDITOR_NO_EXPORT
    SaveResult saveBufferUnprivileged(const QString &filename);

    /**
     * Attempt to save the buffer content in the given filename location using
     * escalated privileges.
     */
    KTEXTEDITOR_NO_EXPORT
    bool saveBufferEscalated(const QString &filename);

public:
    /**
     * Gets the document to which this buffer is bound.
     * \return a pointer to the document
     */
    KTextEditor::DocumentPrivate *document() const
    {
        return m_document;
    }

    /**
     * Debug output, print whole buffer content with line numbers and line length
     * @param title title for this output
     */
    void debugPrint(const QString &title) const;

    /**
     * Return the ranges which affect the given line.
     * @param line line to look at
     * @param view only return ranges associated with given view
     * @param rangesWithAttributeOnly only return ranges which have a attribute set
     * @return list of ranges affecting this line
     */
    QList<TextRange *> rangesForLine(int line, KTextEditor::View *view, bool rangesWithAttributeOnly) const
    {
        // get block, this will assert on invalid line
        const int blockIndex = blockForLine(line);
        return m_blocks.at(blockIndex)->rangesForLine(line, view, rangesWithAttributeOnly);
    }

    void rangesForLine(int line, KTextEditor::View *view, bool rangesWithAttributeOnly, QList<TextRange *> &outRanges) const
    {
        // get block, this will assert on invalid line
        const int blockIndex = blockForLine(line);
        return m_blocks.at(blockIndex)->rangesForLine(line, view, rangesWithAttributeOnly, outRanges);
    }

    /**
     * Check if the given range pointer is still valid.
     * @return range pointer still belongs to range for this buffer
     */
    bool rangePointerValid(TextRange *range) const
    {
        return m_ranges.contains(range);
    }

    /**
     * Invalidate all ranges in this buffer.
     */
    void invalidateRanges();

    //
    // checksum handling
    //
public:
    /**
     * Checksum of the document on disk, set either through file loading
     * in openFile() or in KTextEditor::DocumentPrivate::saveFile()
     * @return git compatible sha1 checksum for this document
     */
    const QByteArray &digest() const;

    /**
     * Set the checksum of this buffer. Make sure this checksum is up-to-date
     * when reading digest().
     * @param checksum git compatible sha1 digest for the document on disk
     */
    void setDigest(const QByteArray &checksum);

private:
    QByteArray m_digest;

private:
    /**
     * parent document
     */
    KTextEditor::DocumentPrivate *m_document;

    /**
     * text history
     */
    TextHistory m_history;

    /**
     * List of blocks which contain the lines of this buffer
     */
    std::vector<TextBlock *> m_blocks;

    /**
     * Number of lines in buffer
     */
    int m_lines;

    /**
     * Revision of the buffer.
     */
    qint64 m_revision;

    /**
     * Current number of running edit transactions
     */
    int m_editingTransactions;

    /**
     * Revision remembered at start of current editing transaction
     */
    qint64 m_editingLastRevision;

    /**
     * Number of lines remembered at start of current editing transaction
     */
    int m_editingLastLines;

    /**
     * minimal line number changed by last editing transaction
     */
    int m_editingMinimalLineChanged;

    /**
     * maximal line number changed by last editing transaction
     */
    int m_editingMaximalLineChanged;

    /**
     * Set of invalid cursors for this whole buffer.
     * Valid cursors are inside the block the belong to.
     */
    QSet<TextCursor *> m_invalidCursors;

    /**
     * Set of ranges of this whole buffer.
     */
    QSet<TextRange *> m_ranges;

    /**
     * Encoding prober type to use
     */
    KEncodingProber::ProberType m_encodingProberType;

    /**
     * Fallback text codec to use
     */
    QString m_fallbackTextCodec;

    /**
     * Text codec to use
     */
    QString m_textCodec;

    /**
     * Mime-Type used for transparent compression/decompression support
     * Set by load(), reset by clear()
     */
    QString m_mimeTypeForFilterDev;

    /**
     * Should byte order mark be created?
     */
    bool m_generateByteOrderMark;

    /**
     * End of line mode, default is Unix
     */
    EndOfLineMode m_endOfLineMode;

    /**
     * Limit for line length, longer lines will be wrapped on load
     */
    int m_lineLengthLimit;

    /**
     * For unit-testing purposes only.
     */
    bool m_alwaysUseKAuthForSave;

    /**
     * For copying QBuffer -> QTemporaryFile while saving document in privileged mode
     */
    static const qint64 bufferLength = 4096;
};

}

#endif
