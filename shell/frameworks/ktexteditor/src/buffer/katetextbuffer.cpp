/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "config.h"

#include "katetextbuffer.h"
#include "katetextloader.h"

#include "katedocument.h"

// this is unfortunate, but needed for performance
#include "katepartdebug.h"
#include "kateview.h"

#ifndef Q_OS_WIN
#include <cerrno>
#include <unistd.h>
// sadly there seems to be no possibility in Qt to determine detailed error
// codes about e.g. file open errors, so we need to resort to evaluating
// errno directly on platforms that support this
#define CAN_USE_ERRNO
#endif

#include <QBuffer>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringEncoder>
#include <QTemporaryFile>

#if HAVE_KAUTH
#include "katesecuretextbuffer_p.h"
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#endif

#if 0
#define BUFFER_DEBUG qCDebug(LOG_KTE)
#else
#define BUFFER_DEBUG                                                                                                                                           \
    if (0)                                                                                                                                                     \
    qCDebug(LOG_KTE)
#endif

namespace Kate
{
TextBuffer::TextBuffer(KTextEditor::DocumentPrivate *parent, bool alwaysUseKAuth)
    : QObject(parent)
    , m_document(parent)
    , m_history(*this)
    , m_lines(0)
    , m_revision(0)
    , m_editingTransactions(0)
    , m_editingLastRevision(0)
    , m_editingLastLines(0)
    , m_editingMinimalLineChanged(-1)
    , m_editingMaximalLineChanged(-1)
    , m_encodingProberType(KEncodingProber::Universal)
    , m_generateByteOrderMark(false)
    , m_endOfLineMode(eolUnix)
    , m_lineLengthLimit(4096)
    , m_alwaysUseKAuthForSave(alwaysUseKAuth)
{
    // create initial state
    clear();
}

TextBuffer::~TextBuffer()
{
    // remove document pointer, this will avoid any notifyAboutRangeChange to have a effect
    m_document = nullptr;

    // not allowed during editing
    Q_ASSERT(m_editingTransactions == 0);

    // kill all ranges, work on copy, they will remove themself from the hash
    QSet<TextRange *> copyRanges = m_ranges;
    qDeleteAll(copyRanges);
    Q_ASSERT(m_ranges.empty());

    // clean out all cursors and lines, only cursors belonging to range will survive
    for (TextBlock *block : std::as_const(m_blocks)) {
        block->deleteBlockContent();
    }

    // delete all blocks, now that all cursors are really deleted
    // else asserts in destructor of blocks will fail!
    qDeleteAll(m_blocks);
    m_blocks.clear();

    // kill all invalid cursors, do this after block deletion, to uncover if they might be still linked in blocks
    QSet<TextCursor *> copyCursors = m_invalidCursors;
    qDeleteAll(copyCursors);
    Q_ASSERT(m_invalidCursors.empty());
}

void TextBuffer::invalidateRanges()
{
    // invalidate all ranges, work on copy, they might delete themself...
    const QSet<TextRange *> copyRanges = m_ranges;
    for (TextRange *range : copyRanges) {
        range->setRange(KTextEditor::Cursor::invalid(), KTextEditor::Cursor::invalid());
    }
}

void TextBuffer::clear()
{
    // not allowed during editing
    Q_ASSERT(m_editingTransactions == 0);

    invalidateRanges();

    // new block for empty buffer
    TextBlock *newBlock = new TextBlock(this, 0);
    newBlock->appendLine(QString());

    // clean out all cursors and lines, either move them to newBlock or invalidate them, if belonging to a range
    for (TextBlock *block : std::as_const(m_blocks)) {
        block->clearBlockContent(newBlock);
    }

    // kill all buffer blocks
    qDeleteAll(m_blocks);
    m_blocks.clear();

    // insert one block with one empty line
    m_blocks.push_back(newBlock);

    // reset lines and last used block
    m_lines = 1;

    // reset revision
    m_revision = 0;

    // reset bom detection
    m_generateByteOrderMark = false;

    // reset the filter device
    m_mimeTypeForFilterDev = QStringLiteral("text/plain");

    // clear edit history
    m_history.clear();

    // we got cleared
    Q_EMIT cleared();
}

TextLine TextBuffer::line(int line) const
{
    // get block, this will assert on invalid line
    int blockIndex = blockForLine(line);

    // get line
    return m_blocks.at(blockIndex)->line(line);
}

void TextBuffer::setLineMetaData(int line, const TextLine &textLine)
{
    // get block, this will assert on invalid line
    int blockIndex = blockForLine(line);

    // get line
    return m_blocks.at(blockIndex)->setLineMetaData(line, textLine);
}

int TextBuffer::cursorToOffset(KTextEditor::Cursor c) const
{
    if (!c.isValid() || c > document()->documentEnd()) {
        return -1;
    }

    int off = 0;
    for (auto block : m_blocks) {
        if (block->startLine() + block->lines() < c.line()) {
            off += block->blockSize();
        } else {
            int start = block->startLine();
            int end = start + block->lines();
            for (int line = start; line < end; ++line) {
                if (line >= c.line()) {
                    off += qMin(c.column(), block->lineLength(line));
                    return off;
                }
                off += block->lineLength(line) + 1;
            }
        }
    }

    Q_ASSERT(false);
    return -1;
}

KTextEditor::Cursor TextBuffer::offsetToCursor(int offset) const
{
    if (offset >= 0) {
        int off = 0;
        for (auto block : m_blocks) {
            if (off + block->blockSize() < offset) {
                off += block->blockSize();
            } else {
                const int lines = block->lines();
                int start = block->startLine();
                int end = start + lines;
                for (int line = start; line < end; ++line) {
                    const int len = block->lineLength(line);
                    if (off + len >= offset) {
                        return KTextEditor::Cursor(line, offset - off);
                    }
                    off += len + 1;
                }
            }
        }
    }
    return KTextEditor::Cursor::invalid();
}

QString TextBuffer::text() const
{
    QString text;
    qsizetype size = 0;
    for (auto b : m_blocks) {
        size += b->blockSize();
    }
    size -= 1; // remove -1, last newline
    text.reserve(size);

    // combine all blocks
    for (TextBlock *block : std::as_const(m_blocks)) {
        block->text(text);
    }

    Q_ASSERT(size == text.size());
    return text;
}

bool TextBuffer::startEditing()
{
    // increment transaction counter
    ++m_editingTransactions;

    // if not first running transaction, do nothing
    if (m_editingTransactions > 1) {
        return false;
    }

    // reset information about edit...
    m_editingLastRevision = m_revision;
    m_editingLastLines = m_lines;
    m_editingMinimalLineChanged = -1;
    m_editingMaximalLineChanged = -1;

    // transaction has started
    Q_EMIT m_document->KTextEditor::Document::editingStarted(m_document);

    // first transaction started
    return true;
}

bool TextBuffer::finishEditing()
{
    // only allowed if still transactions running
    Q_ASSERT(m_editingTransactions > 0);

    // decrement counter
    --m_editingTransactions;

    // if not last running transaction, do nothing
    if (m_editingTransactions > 0) {
        return false;
    }

    // assert that if buffer changed, the line ranges are set and valid!
    Q_ASSERT(!editingChangedBuffer() || (m_editingMinimalLineChanged != -1 && m_editingMaximalLineChanged != -1));
    Q_ASSERT(!editingChangedBuffer() || (m_editingMinimalLineChanged <= m_editingMaximalLineChanged));
    Q_ASSERT(!editingChangedBuffer() || (m_editingMinimalLineChanged >= 0 && m_editingMinimalLineChanged < m_lines));
    Q_ASSERT(!editingChangedBuffer() || (m_editingMaximalLineChanged >= 0 && m_editingMaximalLineChanged < m_lines));

    // transaction has finished
    Q_EMIT m_document->KTextEditor::Document::editingFinished(m_document);

    // last transaction finished
    return true;
}

void TextBuffer::wrapLine(const KTextEditor::Cursor position)
{
    // debug output for REAL low-level debugging
    BUFFER_DEBUG << "wrapLine" << position;

    // only allowed if editing transaction running
    Q_ASSERT(m_editingTransactions > 0);

    // get block, this will assert on invalid line
    int blockIndex = blockForLine(position.line());

    // let the block handle the wrapLine
    // this can only lead to one more line in this block
    // no other blocks will change
    // this call will trigger fixStartLines
    ++m_lines; // first alter the line counter, as functions called will need the valid one
    m_blocks.at(blockIndex)->wrapLine(position, blockIndex);

    // remember changes
    ++m_revision;

    // update changed line interval
    if (position.line() < m_editingMinimalLineChanged || m_editingMinimalLineChanged == -1) {
        m_editingMinimalLineChanged = position.line();
    }

    if (position.line() <= m_editingMaximalLineChanged) {
        ++m_editingMaximalLineChanged;
    } else {
        m_editingMaximalLineChanged = position.line() + 1;
    }

    // balance the changed block if needed
    balanceBlock(blockIndex);

    // emit signal about done change
    Q_EMIT m_document->KTextEditor::Document::lineWrapped(m_document, position);
}

void TextBuffer::unwrapLine(int line)
{
    // debug output for REAL low-level debugging
    BUFFER_DEBUG << "unwrapLine" << line;

    // only allowed if editing transaction running
    Q_ASSERT(m_editingTransactions > 0);

    // line 0 can't be unwrapped
    Q_ASSERT(line > 0);

    // get block, this will assert on invalid line
    int blockIndex = blockForLine(line);

    // is this the first line in the block?
    bool firstLineInBlock = (line == m_blocks.at(blockIndex)->startLine());

    // let the block handle the unwrapLine
    // this can either lead to one line less in this block or the previous one
    // the previous one could even end up with zero lines
    // this call will trigger fixStartLines
    m_blocks.at(blockIndex)->unwrapLine(line, (blockIndex > 0) ? m_blocks.at(blockIndex - 1) : nullptr, firstLineInBlock ? (blockIndex - 1) : blockIndex);
    --m_lines;

    // decrement index for later fixup, if we modified the block in front of the found one
    if (firstLineInBlock) {
        --blockIndex;
    }

    // remember changes
    ++m_revision;

    // update changed line interval
    if ((line - 1) < m_editingMinimalLineChanged || m_editingMinimalLineChanged == -1) {
        m_editingMinimalLineChanged = line - 1;
    }

    if (line <= m_editingMaximalLineChanged) {
        --m_editingMaximalLineChanged;
    } else {
        m_editingMaximalLineChanged = line - 1;
    }

    // balance the changed block if needed
    balanceBlock(blockIndex);

    // emit signal about done change
    Q_EMIT m_document->KTextEditor::Document::lineUnwrapped(m_document, line);
}

void TextBuffer::insertText(const KTextEditor::Cursor position, const QString &text)
{
    // debug output for REAL low-level debugging
    BUFFER_DEBUG << "insertText" << position << text;

    // only allowed if editing transaction running
    Q_ASSERT(m_editingTransactions > 0);

    // skip work, if no text to insert
    if (text.isEmpty()) {
        return;
    }

    // get block, this will assert on invalid line
    int blockIndex = blockForLine(position.line());

    // let the block handle the insertText
    m_blocks.at(blockIndex)->insertText(position, text);

    // remember changes
    ++m_revision;

    // update changed line interval
    if (position.line() < m_editingMinimalLineChanged || m_editingMinimalLineChanged == -1) {
        m_editingMinimalLineChanged = position.line();
    }

    if (position.line() > m_editingMaximalLineChanged) {
        m_editingMaximalLineChanged = position.line();
    }

    // emit signal about done change
    Q_EMIT m_document->KTextEditor::Document::textInserted(m_document, position, text);
}

void TextBuffer::removeText(KTextEditor::Range range)
{
    // debug output for REAL low-level debugging
    BUFFER_DEBUG << "removeText" << range;

    // only allowed if editing transaction running
    Q_ASSERT(m_editingTransactions > 0);

    // only ranges on one line are supported
    Q_ASSERT(range.start().line() == range.end().line());

    // start column <= end column and >= 0
    Q_ASSERT(range.start().column() <= range.end().column());
    Q_ASSERT(range.start().column() >= 0);

    // skip work, if no text to remove
    if (range.isEmpty()) {
        return;
    }

    // get block, this will assert on invalid line
    int blockIndex = blockForLine(range.start().line());

    // let the block handle the removeText, retrieve removed text
    QString text;
    m_blocks.at(blockIndex)->removeText(range, text);

    // remember changes
    ++m_revision;

    // update changed line interval
    if (range.start().line() < m_editingMinimalLineChanged || m_editingMinimalLineChanged == -1) {
        m_editingMinimalLineChanged = range.start().line();
    }

    if (range.start().line() > m_editingMaximalLineChanged) {
        m_editingMaximalLineChanged = range.start().line();
    }

    // emit signal about done change
    Q_EMIT m_document->KTextEditor::Document::textRemoved(m_document, range, text);
}

int TextBuffer::blockForLine(int line) const
{
    // only allow valid lines
    if ((line < 0) || (line >= lines())) {
        qFatal("out of range line requested in text buffer (%d out of [0, %d])", line, lines());
    }

    size_t b = line / BufferBlockSize;
    if (b >= m_blocks.size()) {
        b = m_blocks.size() - 1;
    }

    auto block = m_blocks[b];
    if (block->startLine() <= line && line < block->startLine() + block->lines()) {
        return b;
    }

    if (block->startLine() > line) {
        for (int i = b - 1; i >= 0; --i) {
            auto block = m_blocks[i];
            if (block->startLine() <= line && line < block->startLine() + block->lines()) {
                return i;
            }
        }
    }

    if (block->startLine() < line || (block->lines() == 0)) {
        for (size_t i = b + 1; i < m_blocks.size(); ++i) {
            auto block = m_blocks[i];
            if (block->startLine() <= line && line < block->startLine() + block->lines()) {
                return i;
            }
        }
    }

    qFatal("line requested in text buffer (%d out of [0, %d[), no block found", line, lines());
    return -1;
}

void TextBuffer::fixStartLines(int startBlock)
{
    // only allow valid start block
    Q_ASSERT(startBlock >= 0);
    Q_ASSERT(startBlock < (int)m_blocks.size());

    // new start line for next block
    TextBlock *block = m_blocks.at(startBlock);
    int newStartLine = block->startLine() + block->lines();

    // fixup block
    for (size_t index = startBlock + 1; index < m_blocks.size(); ++index) {
        // set new start line
        block = m_blocks.at(index);
        block->setStartLine(newStartLine);

        // calculate next start line
        newStartLine += block->lines();
    }
}

void TextBuffer::balanceBlock(int index)
{
    // two cases, too big or too small block
    TextBlock *blockToBalance = m_blocks.at(index);

    // first case, too big one, split it
    if (blockToBalance->lines() >= 2 * BufferBlockSize) {
        // half the block
        int halfSize = blockToBalance->lines() / 2;

        // create and insert new block behind current one, already set right start line
        TextBlock *newBlock = blockToBalance->splitBlock(halfSize);
        Q_ASSERT(newBlock);
        m_blocks.insert(m_blocks.begin() + index + 1, newBlock);

        // split is done
        return;
    }

    // second case: possibly too small block

    // if only one block, no chance to unite
    // same if this is first block, we always append to previous one
    if (index == 0) {
        return;
    }

    // block still large enough, do nothing
    if (2 * blockToBalance->lines() > BufferBlockSize) {
        return;
    }

    // unite small block with predecessor
    TextBlock *targetBlock = m_blocks.at(index - 1);

    // merge block
    blockToBalance->mergeBlock(targetBlock);

    // delete old block
    delete blockToBalance;
    m_blocks.erase(m_blocks.begin() + index);
}

void TextBuffer::debugPrint(const QString &title) const
{
    // print header with title
    printf("%s (lines: %d)\n", qPrintable(title), m_lines);

    // print all blocks
    for (size_t i = 0; i < m_blocks.size(); ++i) {
        m_blocks.at(i)->debugPrint(i);
    }
}

bool TextBuffer::load(const QString &filename, bool &encodingErrors, bool &tooLongLinesWrapped, int &longestLineLoaded, bool enforceTextCodec)
{
    // fallback codec must exist
    Q_ASSERT(!m_fallbackTextCodec.isEmpty());

    // codec must be set!
    Q_ASSERT(!m_textCodec.isEmpty());

    // first: clear buffer in any case!
    clear();

    // construct the file loader for the given file, with correct prober type
    Kate::TextLoader file(filename, m_encodingProberType, m_lineLengthLimit);

    // triple play, maximal three loading rounds
    // 0) use the given encoding, be done, if no encoding errors happen
    // 1) use BOM to decided if Unicode or if that fails, use encoding prober, if no encoding errors happen, be done
    // 2) use fallback encoding, be done, if no encoding errors happen
    // 3) use again given encoding, be done in any case
    for (int i = 0; i < (enforceTextCodec ? 1 : 4); ++i) {
        // kill all blocks beside first one
        for (size_t b = 1; b < m_blocks.size(); ++b) {
            TextBlock *block = m_blocks.at(b);
            block->clearLines();
            delete block;
        }
        m_blocks.resize(1);

        // remove lines in first block
        m_blocks.back()->clearLines();
        m_lines = 0;

        // reset error flags
        tooLongLinesWrapped = false;
        longestLineLoaded = 0;

        // try to open file, with given encoding
        // in round 0 + 3 use the given encoding from user
        // in round 1 use 0, to trigger detection
        // in round 2 use fallback
        QString codec = m_textCodec;
        if (i == 1) {
            codec.clear();
        } else if (i == 2) {
            codec = m_fallbackTextCodec;
        }

        if (!file.open(codec)) {
            // create one dummy textline, in any case
            m_blocks.back()->appendLine(QString());
            m_lines++;
            return false;
        }

        // read in all lines...
        encodingErrors = false;
        while (!file.eof()) {
            // read line
            int offset = 0;
            int length = 0;
            bool currentError = !file.readLine(offset, length, tooLongLinesWrapped, longestLineLoaded);
            encodingErrors = encodingErrors || currentError;

            // bail out on encoding error, if not last round!
            if (encodingErrors && i < (enforceTextCodec ? 0 : 3)) {
                BUFFER_DEBUG << "Failed try to load file" << filename << "with codec" << file.textCodec();
                break;
            }

            // ensure blocks aren't too large
            if (m_blocks.back()->lines() >= BufferBlockSize) {
                m_blocks.push_back(new TextBlock(this, m_blocks.back()->startLine() + m_blocks.back()->lines()));
            }

            // append line to last block
            m_blocks.back()->appendLine(QString(file.unicode() + offset, length));
            ++m_lines;
        }

        // if no encoding error, break out of reading loop
        if (!encodingErrors) {
            // remember used codec, might change bom setting
            setTextCodec(file.textCodec());
            break;
        }
    }

    // save checksum of file on disk
    setDigest(file.digest());

    // remember if BOM was found
    if (file.byteOrderMarkFound()) {
        setGenerateByteOrderMark(true);
    }

    // remember eol mode, if any found in file
    if (file.eol() != eolUnknown) {
        setEndOfLineMode(file.eol());
    }

    // remember mime type for filter device
    m_mimeTypeForFilterDev = file.mimeTypeForFilterDev();

    // assert that one line is there!
    Q_ASSERT(m_lines > 0);

    // report CODEC + ERRORS
    BUFFER_DEBUG << "Loaded file " << filename << "with codec" << m_textCodec << (encodingErrors ? "with" : "without") << "encoding errors";

    // report BOM
    BUFFER_DEBUG << (file.byteOrderMarkFound() ? "Found" : "Didn't find") << "byte order mark";

    // report filter device mime-type
    BUFFER_DEBUG << "used filter device for mime-type" << m_mimeTypeForFilterDev;

    // emit success
    Q_EMIT loaded(filename, encodingErrors);

    // file loading worked, modulo encoding problems
    return true;
}

const QByteArray &TextBuffer::digest() const
{
    return m_digest;
}

void TextBuffer::setDigest(const QByteArray &checksum)
{
    m_digest = checksum;
}

void TextBuffer::setTextCodec(const QString &codec)
{
    m_textCodec = codec;

    // enforce bom for some encodings
    if (const auto setEncoding = QStringConverter::encodingForName(m_textCodec.toUtf8().constData())) {
        for (const auto encoding : {QStringConverter::Utf16,
                                    QStringConverter::Utf16BE,
                                    QStringConverter::Utf16LE,
                                    QStringConverter::Utf32,
                                    QStringConverter::Utf32BE,
                                    QStringConverter::Utf32LE}) {
            if (setEncoding == encoding) {
                setGenerateByteOrderMark(true);
                break;
            }
        }
    }
}

bool TextBuffer::save(const QString &filename)
{
    // codec must be set, else below we fail!
    Q_ASSERT(!m_textCodec.isEmpty());

    SaveResult saveRes = saveBufferUnprivileged(filename);

    if (saveRes == SaveResult::Failed) {
        return false;
    } else if (saveRes == SaveResult::MissingPermissions) {
        // either unit-test mode or we're missing permissions to write to the
        // file => use temporary file and try to use authhelper
        if (!saveBufferEscalated(filename)) {
            return false;
        }
    }

    // remember this revision as last saved
    m_history.setLastSavedRevision();

    // inform that we have saved the state
    markModifiedLinesAsSaved();

    // emit that file was saved and be done
    Q_EMIT saved(filename);
    return true;
}

bool TextBuffer::saveBuffer(const QString &filename, KCompressionDevice &saveFile)
{
    QStringEncoder encoder(m_textCodec.toUtf8().constData(), generateByteOrderMark() ? QStringConverter::Flag::WriteBom : QStringConverter::Flag::Default);

    // our loved eol string ;)
    QString eol = QStringLiteral("\n");
    if (endOfLineMode() == eolDos) {
        eol = QStringLiteral("\r\n");
    } else if (endOfLineMode() == eolMac) {
        eol = QStringLiteral("\r");
    }

    // just dump the lines out ;)
    for (int i = 0; i < m_lines; ++i) {
        // dump current line
        saveFile.write(encoder.encode(line(i).text()));

        // append correct end of line string
        if ((i + 1) < m_lines) {
            saveFile.write(encoder.encode(eol));
        }

        // early out on stream errors
        if (saveFile.error() != QFileDevice::NoError) {
            return false;
        }
    }

    // TODO: this only writes bytes when there is text. This is a fine optimization for most cases, but this makes saving
    // an empty file with the BOM set impossible (results to an empty file with 0 bytes, no BOM)

    // close the file, we might want to read from underlying buffer below
    saveFile.close();

    // did save work?
    if (saveFile.error() != QFileDevice::NoError) {
        BUFFER_DEBUG << "Saving file " << filename << "failed with error" << saveFile.errorString();
        return false;
    }

    return true;
}

TextBuffer::SaveResult TextBuffer::saveBufferUnprivileged(const QString &filename)
{
    if (m_alwaysUseKAuthForSave) {
        // unit-testing mode, simulate we need privileges
        return SaveResult::MissingPermissions;
    }

    // construct correct filter device
    // we try to use the same compression as for opening
    const KCompressionDevice::CompressionType type = KCompressionDevice::compressionTypeForMimeType(m_mimeTypeForFilterDev);
    auto saveFile = std::make_unique<KCompressionDevice>(filename, type);

    if (!saveFile->open(QIODevice::WriteOnly)) {
#ifdef CAN_USE_ERRNO
        if (errno != EACCES) {
            return SaveResult::Failed;
        }
#endif
        return SaveResult::MissingPermissions;
    }

    if (!saveBuffer(filename, *saveFile)) {
        return SaveResult::Failed;
    }

    return SaveResult::Success;
}

bool TextBuffer::saveBufferEscalated(const QString &filename)
{
#if HAVE_KAUTH
    // construct correct filter device
    // we try to use the same compression as for opening
    const KCompressionDevice::CompressionType type = KCompressionDevice::compressionTypeForMimeType(m_mimeTypeForFilterDev);
    auto saveFile = std::make_unique<KCompressionDevice>(filename, type);
    uint ownerId = -2;
    uint groupId = -2;
    std::unique_ptr<QIODevice> temporaryBuffer;

    // Memorize owner and group.
    const QFileInfo fileInfo(filename);
    if (fileInfo.exists()) {
        ownerId = fileInfo.ownerId();
        groupId = fileInfo.groupId();
    }

    // if that fails we need more privileges to save this file
    // -> we write to a temporary file and then send its path to KAuth action for privileged save
    temporaryBuffer = std::make_unique<QBuffer>();

    // open buffer for write and read (read is used for checksum computing and writing to temporary file)
    if (!temporaryBuffer->open(QIODevice::ReadWrite)) {
        return false;
    }

    // we are now saving to a temporary buffer with potential compression proxy
    saveFile = std::make_unique<KCompressionDevice>(temporaryBuffer.get(), false, type);
    if (!saveFile->open(QIODevice::WriteOnly)) {
        return false;
    }

    if (!saveBuffer(filename, *saveFile)) {
        return false;
    }

    // temporary buffer was used to save the file
    // -> computing checksum
    // -> saving to temporary file
    // -> copying the temporary file to the original file location with KAuth action
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        return false;
    }

    // go to QBuffer start
    temporaryBuffer->seek(0);

    // read contents of QBuffer and add them to checksum utility as well as to QTemporaryFile
    char buffer[bufferLength];
    qint64 read = -1;
    QCryptographicHash cryptographicHash(SecureTextBuffer::checksumAlgorithm);
    while ((read = temporaryBuffer->read(buffer, bufferLength)) > 0) {
        cryptographicHash.addData(QByteArrayView(buffer, read));
        if (tempFile.write(buffer, read) == -1) {
            return false;
        }
    }
    if (!tempFile.flush()) {
        return false;
    }

    // prepare data for KAuth action
    QVariantMap kAuthActionArgs;
    kAuthActionArgs.insert(QStringLiteral("sourceFile"), tempFile.fileName());
    kAuthActionArgs.insert(QStringLiteral("targetFile"), filename);
    kAuthActionArgs.insert(QStringLiteral("checksum"), cryptographicHash.result());
    kAuthActionArgs.insert(QStringLiteral("ownerId"), ownerId);
    kAuthActionArgs.insert(QStringLiteral("groupId"), groupId);

    // call save with elevated privileges
    if (QStandardPaths::isTestModeEnabled()) {
        // unit testing purposes only
        if (!SecureTextBuffer::savefile(kAuthActionArgs).succeeded()) {
            return false;
        }
    } else {
        KAuth::Action kAuthSaveAction(QStringLiteral("org.kde.ktexteditor6.katetextbuffer.savefile"));
        kAuthSaveAction.setHelperId(QStringLiteral("org.kde.ktexteditor6.katetextbuffer"));
        kAuthSaveAction.setArguments(kAuthActionArgs);
        KAuth::ExecuteJob *job = kAuthSaveAction.execute();
        if (!job->exec()) {
            return false;
        }
    }

    return true;
#else
    Q_UNUSED(filename);
    return false;
#endif
}

void TextBuffer::notifyAboutRangeChange(KTextEditor::View *view, KTextEditor::LineRange lineRange, bool needsRepaint)
{
    // ignore calls if no document is around
    if (!m_document) {
        return;
    }

    // update all views, this IS ugly and could be a signal, but I profiled and a signal is TOO slow, really
    // just create 20k ranges in a go and you wait seconds on a decent machine
    const QList<KTextEditor::View *> views = m_document->views();
    for (KTextEditor::View *curView : views) {
        // filter wrong views
        if (view && view != curView) {
            continue;
        }

        // notify view, it is really a kate view
        static_cast<KTextEditor::ViewPrivate *>(curView)->notifyAboutRangeChange(lineRange, needsRepaint);
    }
}

void TextBuffer::markModifiedLinesAsSaved()
{
    for (TextBlock *block : std::as_const(m_blocks)) {
        block->markModifiedLinesAsSaved();
    }
}
}

#include "moc_katetextbuffer.cpp"
