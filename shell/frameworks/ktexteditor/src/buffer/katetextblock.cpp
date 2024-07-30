/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katetextblock.h"
#include "katetextbuffer.h"
#include "katetextcursor.h"
#include "katetextrange.h"

namespace Kate
{
TextBlock::TextBlock(TextBuffer *buffer, int startLine)
    : m_buffer(buffer)
    , m_startLine(startLine)
{
    // reserve the block size
    m_lines.reserve(BufferBlockSize);
}

TextBlock::~TextBlock()
{
    // blocks should be empty before they are deleted!
    Q_ASSERT(m_blockSize == 0);
    Q_ASSERT(m_lines.empty());
    Q_ASSERT(m_cursors.empty());

    // it only is a hint for ranges for this block, not the storage of them
}

void TextBlock::setStartLine(int startLine)
{
    // allow only valid lines
    Q_ASSERT(startLine >= 0);
    Q_ASSERT(startLine < m_buffer->lines());

    m_startLine = startLine;
}

TextLine TextBlock::line(int line) const
{
    // right input
    Q_ASSERT(line >= startLine());

    // get text line, at will bail out on out-of-range
    return m_lines.at(line - startLine());
}

void TextBlock::setLineMetaData(int line, const TextLine &textLine)
{
    // right input
    Q_ASSERT(line >= startLine());

    // set stuff, at will bail out on out-of-range
    const QString originalText = m_lines.at(line - startLine()).text();
    m_lines.at(line - startLine()) = textLine;
    m_lines.at(line - startLine()).text() = originalText;
}

void TextBlock::appendLine(const QString &textOfLine)
{
    m_lines.emplace_back(textOfLine);
    m_blockSize += textOfLine.size();
}

void TextBlock::clearLines()
{
    m_lines.clear();
    m_blockSize = 0;
}

void TextBlock::text(QString &text) const
{
    // combine all lines
    for (size_t i = 0; i < m_lines.size(); ++i) {
        // not first line, insert \n
        if (i > 0 || startLine() > 0) {
            text.append(QLatin1Char('\n'));
        }

        text.append(m_lines.at(i).text());
    }
}

void TextBlock::wrapLine(const KTextEditor::Cursor position, int fixStartLinesStartIndex)
{
    // calc internal line
    int line = position.line() - startLine();

    // get text, copy, we might invalidate the reference
    const QString text = m_lines.at(line).text();

    // check if valid column
    Q_ASSERT(position.column() >= 0);
    Q_ASSERT(position.column() <= text.size());

    // create new line and insert it
    m_lines.insert(m_lines.begin() + line + 1, TextLine());

    // cases for modification:
    // 1. line is wrapped in the middle
    // 2. if empty line is wrapped, mark new line as modified
    // 3. line-to-be-wrapped is already modified
    if (position.column() > 0 || text.size() == 0 || m_lines.at(line).markedAsModified()) {
        m_lines.at(line + 1).markAsModified(true);
    } else if (m_lines.at(line).markedAsSavedOnDisk()) {
        m_lines.at(line + 1).markAsSavedOnDisk(true);
    }

    // perhaps remove some text from previous line and append it
    if (position.column() < text.size()) {
        // text from old line moved first to new one
        m_lines.at(line + 1).text() = text.right(text.size() - position.column());

        // now remove wrapped text from old line
        m_lines.at(line).text().chop(text.size() - position.column());

        // mark line as modified
        m_lines.at(line).markAsModified(true);
    }

    // fix all start lines
    // we need to do this NOW, else the range update will FAIL!
    // bug 313759
    m_buffer->fixStartLines(fixStartLinesStartIndex);

    // notify the text history
    m_buffer->history().wrapLine(position);

    // cursor and range handling below

    // no cursors will leave or join this block

    // no cursors in this block, no work to do..
    if (m_cursors.empty()) {
        return;
    }

    // move all cursors on the line which has the text inserted
    // remember all ranges modified, optimize for the standard case of a few ranges
    QVarLengthArray<TextRange *, 32> changedRanges;
    for (TextCursor *cursor : m_cursors) {
        // skip cursors on lines in front of the wrapped one!
        if (cursor->lineInBlock() < line) {
            continue;
        }

        // either this is simple, line behind the wrapped one
        if (cursor->lineInBlock() > line) {
            // patch line of cursor
            cursor->m_line++;
        }

        // this is the wrapped line
        else {
            // skip cursors with too small column
            if (cursor->column() <= position.column()) {
                if (cursor->column() < position.column() || !cursor->m_moveOnInsert) {
                    continue;
                }
            }

            // move cursor

            // patch line of cursor
            cursor->m_line++;

            // patch column
            cursor->m_column -= position.column();
        }

        // remember range, if any, avoid double insert
        auto range = cursor->kateRange();
        if (range && !range->isValidityCheckRequired()) {
            range->setValidityCheckRequired();
            changedRanges.push_back(range);
        }
    }

    // we might need to invalidate ranges or notify about their changes
    // checkValidity might trigger delete of the range!
    for (TextRange *range : std::as_const(changedRanges)) {
        // we need to do updateRange to ALWAYS ensure the line => range and back cache is updated
        // see MovingRangeTest::testLineWrapOrUnwrapUpdateRangeForLineCache
        updateRange(range);

        // in addition: ensure that we really invalidate bad ranges!
        range->checkValidity(range->toLineRange());
    }
}

void TextBlock::unwrapLine(int line, TextBlock *previousBlock, int fixStartLinesStartIndex)
{
    // calc internal line
    line = line - startLine();

    // two possiblities: either first line of this block or later line
    if (line == 0) {
        // we need previous block with at least one line
        Q_ASSERT(previousBlock);
        Q_ASSERT(previousBlock->lines() > 0);

        // move last line of previous block to this one, might result in empty block
        const TextLine oldFirst = m_lines.at(0);
        int lastLineOfPreviousBlock = previousBlock->lines() - 1;
        m_lines[0] = previousBlock->m_lines.back();
        previousBlock->m_lines.erase(previousBlock->m_lines.begin() + (previousBlock->lines() - 1));

        const int oldSizeOfPreviousLine = m_lines[0].text().size();
        if (oldFirst.length() > 0) {
            // append text
            m_lines[0].text().append(oldFirst.text());

            // mark line as modified, since text was appended
            m_lines[0].markAsModified(true);
        }

        // patch startLine of this block
        --m_startLine;

        // fix all start lines
        // we need to do this NOW, else the range update will FAIL!
        // bug 313759
        m_buffer->fixStartLines(fixStartLinesStartIndex);

        // notify the text history in advance
        m_buffer->history().unwrapLine(startLine() + line, oldSizeOfPreviousLine);

        // cursor and range handling below

        // no cursors in this block and the previous one, no work to do..
        if (m_cursors.empty() && previousBlock->m_cursors.empty()) {
            return;
        }

        // move all cursors because of the unwrapped line
        // remember all ranges modified, optimize for the standard case of a few ranges
        QVarLengthArray<TextRange *, 32> changedRanges;
        for (TextCursor *cursor : m_cursors) {
            // this is the unwrapped line
            if (cursor->lineInBlock() == 0) {
                // patch column
                cursor->m_column += oldSizeOfPreviousLine;

                // remember range, if any, avoid double insert
                auto range = cursor->kateRange();
                if (range && !range->isValidityCheckRequired()) {
                    range->setValidityCheckRequired();
                    changedRanges.push_back(range);
                }
            }
        }

        // move cursors of the moved line from previous block to this block now
        for (auto it = previousBlock->m_cursors.begin(); it != previousBlock->m_cursors.end();) {
            auto cursor = *it;
            if (cursor->lineInBlock() == lastLineOfPreviousBlock) {
                cursor->m_line = 0;
                cursor->m_block = this;
                m_cursors.insert(cursor);

                // remember range, if any, avoid double insert
                auto range = cursor->kateRange();
                if (range && !range->isValidityCheckRequired()) {
                    range->setValidityCheckRequired();
                    changedRanges.push_back(range);
                }

                // remove from previous block
                it = previousBlock->m_cursors.erase(it);
            } else {
                // keep in previous block
                ++it;
            }
        }

        // fixup the ranges that might be effected, because they moved from last line to this block
        // we might need to invalidate ranges or notify about their changes
        // checkValidity might trigger delete of the range!
        for (TextRange *range : std::as_const(changedRanges)) {
            // update both blocks
            updateRange(range);
            previousBlock->updateRange(range);

            // afterwards check validity, might delete this range!
            range->checkValidity(range->toLineRange());
        }

        // be done
        return;
    }

    // easy: just move text to previous line and remove current one
    const int oldSizeOfPreviousLine = m_lines.at(line - 1).length();
    const int sizeOfCurrentLine = m_lines.at(line).length();
    if (sizeOfCurrentLine > 0) {
        m_lines.at(line - 1).text().append(m_lines.at(line).text());
    }

    const bool lineChanged = (oldSizeOfPreviousLine > 0 && m_lines.at(line - 1).markedAsModified())
        || (sizeOfCurrentLine > 0 && (oldSizeOfPreviousLine > 0 || m_lines.at(line).markedAsModified()));
    m_lines.at(line - 1).markAsModified(lineChanged);
    if (oldSizeOfPreviousLine == 0 && m_lines.at(line).markedAsSavedOnDisk()) {
        m_lines.at(line - 1).markAsSavedOnDisk(true);
    }

    m_lines.erase(m_lines.begin() + line);

    // fix all start lines
    // we need to do this NOW, else the range update will FAIL!
    // bug 313759
    m_buffer->fixStartLines(fixStartLinesStartIndex);

    // notify the text history in advance
    m_buffer->history().unwrapLine(startLine() + line, oldSizeOfPreviousLine);

    // cursor and range handling below

    // no cursors in this block, no work to do..
    if (m_cursors.empty()) {
        return;
    }

    // move all cursors because of the unwrapped line
    // remember all ranges modified, optimize for the standard case of a few ranges
    QVarLengthArray<TextRange *, 32> changedRanges;
    for (TextCursor *cursor : m_cursors) {
        // skip cursors in lines in front of removed one
        if (cursor->lineInBlock() < line) {
            continue;
        }

        // this is the unwrapped line
        if (cursor->lineInBlock() == line) {
            // patch column
            cursor->m_column += oldSizeOfPreviousLine;
        }

        // patch line of cursor
        cursor->m_line--;

        // remember range, if any, avoid double insert
        auto range = cursor->kateRange();
        if (range && !range->isValidityCheckRequired()) {
            range->setValidityCheckRequired();
            changedRanges.push_back(range);
        }
    }

    // we might need to invalidate ranges or notify about their changes
    // checkValidity might trigger delete of the range!
    for (TextRange *range : std::as_const(changedRanges)) {
        // we need to do updateRange to ALWAYS ensure the line => range and back cache is updated
        // see MovingRangeTest::testLineWrapOrUnwrapUpdateRangeForLineCache
        updateRange(range);

        // in addition: ensure that we really invalidate bad ranges!
        range->checkValidity(range->toLineRange());
    }
}

void TextBlock::insertText(const KTextEditor::Cursor position, const QString &text)
{
    // calc internal line
    int line = position.line() - startLine();

    // get text
    QString &textOfLine = m_lines.at(line).text();
    int oldLength = textOfLine.size();
    m_lines.at(line).markAsModified(true);

    // check if valid column
    Q_ASSERT(position.column() >= 0);
    Q_ASSERT(position.column() <= textOfLine.size());

    // insert text
    textOfLine.insert(position.column(), text);

    // notify the text history
    m_buffer->history().insertText(position, text.size(), oldLength);

    m_blockSize += text.size();

    // cursor and range handling below

    // no cursors in this block, no work to do..
    if (m_cursors.empty()) {
        return;
    }

    // move all cursors on the line which has the text inserted
    // remember all ranges modified, optimize for the standard case of a few ranges
    QVarLengthArray<TextRange *, 32> changedRanges;
    for (TextCursor *cursor : m_cursors) {
        // skip cursors not on this line!
        if (cursor->lineInBlock() != line) {
            continue;
        }

        // skip cursors with too small column
        if (cursor->column() <= position.column()) {
            if (cursor->column() < position.column() || !cursor->m_moveOnInsert) {
                continue;
            }
        }

        // patch column of cursor
        if (cursor->m_column <= oldLength) {
            cursor->m_column += text.size();
        }

        // special handling if cursor behind the real line, e.g. non-wrapping cursor in block selection mode
        else if (cursor->m_column < textOfLine.size()) {
            cursor->m_column = textOfLine.size();
        }

        // remember range, if any, avoid double insert
        // we only need to trigger checkValidity later if the range has feedback or might be invalidated
        auto range = cursor->kateRange();
        if (range && !range->isValidityCheckRequired() && (range->feedback() || range->start().line() == range->end().line())) {
            range->setValidityCheckRequired();
            changedRanges.push_back(range);
        }
    }

    // we might need to invalidate ranges or notify about their changes
    // checkValidity might trigger delete of the range!
    for (TextRange *range : std::as_const(changedRanges)) {
        range->checkValidity(range->toLineRange());
    }
}

void TextBlock::removeText(KTextEditor::Range range, QString &removedText)
{
    // calc internal line
    int line = range.start().line() - startLine();

    // get text
    QString &textOfLine = m_lines.at(line).text();
    int oldLength = textOfLine.size();

    // check if valid column
    Q_ASSERT(range.start().column() >= 0);
    Q_ASSERT(range.start().column() <= textOfLine.size());
    Q_ASSERT(range.end().column() >= 0);
    Q_ASSERT(range.end().column() <= textOfLine.size());

    // get text which will be removed
    removedText = textOfLine.mid(range.start().column(), range.end().column() - range.start().column());

    // remove text
    textOfLine.remove(range.start().column(), range.end().column() - range.start().column());
    m_lines.at(line).markAsModified(true);

    // notify the text history
    m_buffer->history().removeText(range, oldLength);

    m_blockSize -= removedText.size();

    // cursor and range handling below

    // no cursors in this block, no work to do..
    if (m_cursors.empty()) {
        return;
    }

    // move all cursors on the line which has the text removed
    // remember all ranges modified, optimize for the standard case of a few ranges
    QVarLengthArray<TextRange *, 32> changedRanges;
    for (TextCursor *cursor : m_cursors) {
        // skip cursors not on this line!
        if (cursor->lineInBlock() != line) {
            continue;
        }

        // skip cursors with too small column
        if (cursor->column() <= range.start().column()) {
            continue;
        }

        // patch column of cursor
        if (cursor->column() <= range.end().column()) {
            cursor->m_column = range.start().column();
        } else {
            cursor->m_column -= (range.end().column() - range.start().column());
        }

        // remember range, if any, avoid double insert
        // we only need to trigger checkValidity later if the range has feedback or might be invalidated
        auto range = cursor->kateRange();
        if (range && !range->isValidityCheckRequired() && (range->feedback() || range->start().line() == range->end().line())) {
            range->setValidityCheckRequired();
            changedRanges.push_back(range);
        }
    }

    // we might need to invalidate ranges or notify about their changes
    // checkValidity might trigger delete of the range!
    for (TextRange *range : std::as_const(changedRanges)) {
        range->checkValidity(range->toLineRange());
    }
}

void TextBlock::debugPrint(int blockIndex) const
{
    // print all blocks
    for (size_t i = 0; i < m_lines.size(); ++i) {
        printf("%4d - %4llu : %4llu : '%s'\n",
               blockIndex,
               (unsigned long long)startLine() + i,
               (unsigned long long)m_lines.at(i).text().size(),
               qPrintable(m_lines.at(i).text()));
    }
}

TextBlock *TextBlock::splitBlock(int fromLine)
{
    // half the block
    int linesOfNewBlock = lines() - fromLine;

    // create and insert new block
    TextBlock *newBlock = new TextBlock(m_buffer, startLine() + fromLine);

    // move lines
    newBlock->m_lines.reserve(linesOfNewBlock);
    for (size_t i = fromLine; i < m_lines.size(); ++i) {
        auto line = std::move(m_lines[i]);
        m_blockSize -= line.length();
        newBlock->m_blockSize += line.length();
        newBlock->m_lines.push_back(std::move(line));
    }

    m_lines.resize(fromLine);

    // move cursors
    for (auto it = m_cursors.begin(); it != m_cursors.end();) {
        auto cursor = *it;
        if (cursor->lineInBlock() >= fromLine) {
            cursor->m_line = cursor->lineInBlock() - fromLine;
            cursor->m_block = newBlock;

            // add to new, remove from current
            newBlock->m_cursors.insert(cursor);
            it = m_cursors.erase(it);
        } else {
            // keep in current
            ++it;
        }
    }

    // fix ALL ranges!
    // copy is necessary as update range may modify the uncached ranges
    std::vector<TextRange *> allRanges;
    allRanges.reserve(m_uncachedRanges.size() + m_cachedLineForRanges.size());
    std::for_each(m_cachedLineForRanges.keyBegin(), m_cachedLineForRanges.keyEnd(), [&allRanges](TextRange *range) {
        allRanges.push_back(range);
    });
    allRanges.insert(allRanges.end(), m_uncachedRanges.begin(), m_uncachedRanges.end());
    for (TextRange *range : allRanges) {
        // update both blocks
        updateRange(range);
        newBlock->updateRange(range);
    }

    // return the new generated block
    return newBlock;
}

void TextBlock::mergeBlock(TextBlock *targetBlock)
{
    // move cursors, do this first, now still lines() count is correct for target
    for (TextCursor *cursor : m_cursors) {
        cursor->m_line = cursor->lineInBlock() + targetBlock->lines();
        cursor->m_block = targetBlock;
        targetBlock->m_cursors.insert(cursor);
    }
    m_cursors.clear();

    // move lines
    targetBlock->m_lines.reserve(targetBlock->lines() + lines());
    for (size_t i = 0; i < m_lines.size(); ++i) {
        targetBlock->m_lines.push_back(m_lines.at(i));
    }
    targetBlock->m_blockSize += m_blockSize;
    clearLines();

    // fix ALL ranges!
    // copy is necessary as update range may modify the uncached ranges
    std::vector<TextRange *> allRanges;
    allRanges.reserve(m_uncachedRanges.size() + m_cachedLineForRanges.size());
    std::for_each(m_cachedLineForRanges.keyBegin(), m_cachedLineForRanges.keyEnd(), [&allRanges](TextRange *range) {
        allRanges.push_back(range);
    });
    allRanges.insert(allRanges.end(), m_uncachedRanges.begin(), m_uncachedRanges.end());
    for (TextRange *range : allRanges) {
        // update both blocks
        updateRange(range);
        targetBlock->updateRange(range);
    }
}

void TextBlock::deleteBlockContent()
{
    // kill cursors, if not belonging to a range
    // we can do in-place editing of the current set of cursors as
    // we remove them before deleting
    for (auto it = m_cursors.begin(); it != m_cursors.end();) {
        auto cursor = *it;
        if (!cursor->kateRange()) {
            // remove it and advance to next element
            it = m_cursors.erase(it);

            // delete after cursor is gone from the set
            // else the destructor will modify it!
            delete cursor;
        } else {
            // keep this cursor
            ++it;
        }
    }

    // kill lines
    clearLines();
}

void TextBlock::clearBlockContent(TextBlock *targetBlock)
{
    // move cursors, if not belonging to a range
    // we can do in-place editing of the current set of cursors
    for (auto it = m_cursors.begin(); it != m_cursors.end();) {
        auto cursor = *it;
        if (!cursor->kateRange()) {
            cursor->m_column = 0;
            cursor->m_line = 0;
            cursor->m_block = targetBlock;
            targetBlock->m_cursors.insert(cursor);

            // remove it and advance to next element
            it = m_cursors.erase(it);
        } else {
            // keep this cursor
            ++it;
        }
    }

    // kill lines
    clearLines();
}

QList<TextRange *> TextBlock::rangesForLine(int line, KTextEditor::View *view, bool rangesWithAttributeOnly) const
{
    const auto cachedRanges = cachedRangesForLine(line);
    QList<TextRange *> ranges;
    ranges.reserve(m_uncachedRanges.size() + (cachedRanges ? cachedRanges->size() : 0));
    rangesForLine(line, view, rangesWithAttributeOnly, ranges);
    return ranges;
}

void TextBlock::rangesForLine(int line, KTextEditor::View *view, bool rangesWithAttributeOnly, QList<TextRange *> &outRanges) const
{
    const auto cachedRanges = cachedRangesForLine(line);
    outRanges.clear();

    auto predicate = [line, view, rangesWithAttributeOnly](TextRange *range) {
        if (rangesWithAttributeOnly && !range->hasAttribute()) {
            return false;
        }

        // we want ranges for no view, but this one's attribute is only valid for views
        if (!view && range->attributeOnlyForViews()) {
            return false;
        }

        // the range's attribute is not valid for this view
        if (range->view() && range->view() != view) {
            return false;
        }

        // if line is in the range, ok
        if (range->startInternal().lineInternal() <= line && line <= range->endInternal().lineInternal()) {
            return true;
        }
        return false;
    };

    if (cachedRanges) {
        std::copy_if(cachedRanges->begin(), cachedRanges->end(), std::back_inserter(outRanges), predicate);
    }
    std::copy_if(m_uncachedRanges.begin(), m_uncachedRanges.end(), std::back_inserter(outRanges), predicate);
}

void TextBlock::markModifiedLinesAsSaved()
{
    // mark all modified lines as saved
    for (auto &textLine : m_lines) {
        if (textLine.markedAsModified()) {
            textLine.markAsSavedOnDisk(true);
        }
    }
}

void TextBlock::updateRange(TextRange *range)
{
    // get some simple facts about our nice range
    const int startLine = range->startInternal().lineInternal();
    const int endLine = range->endInternal().lineInternal();
    const bool isSingleLine = startLine == endLine;

    // perhaps remove range and be done
    if ((endLine < m_startLine) || (startLine >= (m_startLine + lines()))) {
        removeRange(range);
        return;
    }

    // The range is still a single-line range, and is still cached to the correct line.
    if (isSingleLine) {
        auto it = m_cachedLineForRanges.find(range);
        if (it != m_cachedLineForRanges.end() && it.value() == startLine - m_startLine) {
            return;
        }
    }

    // The range is still a multi-line range, and is already in the correct set.
    if (!isSingleLine && m_uncachedRanges.contains(range)) {
        return;
    }

    // remove, if already there!
    removeRange(range);

    // simple case: multi-line range
    if (!isSingleLine) {
        // The range cannot be cached per line, as it spans multiple lines
        m_uncachedRanges.append(range);
        return;
    }

    // The range is contained by a single line, put it into the line-cache
    const int lineOffset = startLine - m_startLine;

    // enlarge cache if needed
    if (m_cachedRangesForLine.size() <= (size_t)lineOffset) {
        m_cachedRangesForLine.resize(lineOffset + 1);
    }

    // insert into mapping
    if (!m_cachedRangesForLine[lineOffset].contains(range)) {
        m_cachedRangesForLine[lineOffset].push_back(range);
    }
    m_cachedLineForRanges[range] = lineOffset;
}

void TextBlock::removeRange(TextRange *range)
{
    // uncached range? remove it and be done
    int pos = m_uncachedRanges.indexOf(range);
    if (pos != -1) {
        m_uncachedRanges.remove(pos);
        // must be only uncached!
        Q_ASSERT(m_cachedLineForRanges.find(range) == m_cachedLineForRanges.end());
        return;
    }

    // cached range?
    auto it = m_cachedLineForRanges.find(range);
    if (it != m_cachedLineForRanges.end()) {
        // must be only cached!
        Q_ASSERT(!m_uncachedRanges.contains(range));

        int line = it.value();

        // query the range from cache, must be there
        int idx = m_cachedRangesForLine[line].indexOf(range);
        Q_ASSERT(idx != -1);

        // remove it and be done
        m_cachedRangesForLine[line].remove(idx);
        m_cachedLineForRanges.erase(it);
        return;
    }

    // else: range was not for this block, just do nothing, removeRange should be "safe" to use
}

}
