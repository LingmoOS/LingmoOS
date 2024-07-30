/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    Based on code of the SmartCursor/Range by:
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katetextrange.h"
#include "katedocument.h"
#include "katetextbuffer.h"

namespace Kate
{
TextRange::TextRange(TextBuffer &buffer, KTextEditor::Range range, InsertBehaviors insertBehavior, EmptyBehavior emptyBehavior)
    : m_buffer(buffer)
    , m_start(buffer, this, range.start(), (insertBehavior & ExpandLeft) ? Kate::TextCursor::StayOnInsert : Kate::TextCursor::MoveOnInsert)
    , m_end(buffer, this, range.end(), (insertBehavior & ExpandRight) ? Kate::TextCursor::MoveOnInsert : Kate::TextCursor::StayOnInsert)
    , m_view(nullptr)
    , m_feedback(nullptr)
    , m_zDepth(0.0)
    , m_attributeOnlyForViews(false)
    , m_invalidateIfEmpty(emptyBehavior == InvalidateIfEmpty)
{
    // remember this range in buffer
    m_buffer.m_ranges.insert(this);

    // check if range now invalid, there can happen no feedback, as m_feedback == 0
    // only place where KTextEditor::LineRange::invalid() for old range makes sense, as we were yet not registered!
    checkValidity(KTextEditor::LineRange::invalid());
}

TextRange::~TextRange()
{
    // reset feedback, don't want feedback during destruction
    m_feedback = nullptr;

    // remove range from m_ranges
    fixLookup(toLineRange(), KTextEditor::LineRange::invalid());

    // remove this range from the buffer
    m_buffer.m_ranges.remove(this);

    // trigger update, if we have attribute
    // notify right view
    // here we can ignore feedback, even with feedback, we want none if the range is deleted!
    if (m_attribute) {
        m_buffer.notifyAboutRangeChange(m_view, toLineRange(), true /* we have a attribute */);
    }
}

void TextRange::setInsertBehaviors(InsertBehaviors _insertBehaviors)
{
    // nothing to do?
    if (_insertBehaviors == insertBehaviors()) {
        return;
    }

    // modify cursors
    m_start.setInsertBehavior((_insertBehaviors & ExpandLeft) ? KTextEditor::MovingCursor::StayOnInsert : KTextEditor::MovingCursor::MoveOnInsert);
    m_end.setInsertBehavior((_insertBehaviors & ExpandRight) ? KTextEditor::MovingCursor::MoveOnInsert : KTextEditor::MovingCursor::StayOnInsert);

    // notify world
    if (m_attribute || m_feedback) {
        m_buffer.notifyAboutRangeChange(m_view, toLineRange(), true /* we have a attribute */);
    }
}

KTextEditor::MovingRange::InsertBehaviors TextRange::insertBehaviors() const
{
    InsertBehaviors behaviors = DoNotExpand;

    if (m_start.insertBehavior() == KTextEditor::MovingCursor::StayOnInsert) {
        behaviors |= ExpandLeft;
    }

    if (m_end.insertBehavior() == KTextEditor::MovingCursor::MoveOnInsert) {
        behaviors |= ExpandRight;
    }

    return behaviors;
}

void TextRange::setEmptyBehavior(EmptyBehavior emptyBehavior)
{
    // nothing to do?
    if (m_invalidateIfEmpty == (emptyBehavior == InvalidateIfEmpty)) {
        return;
    }

    // remember value
    m_invalidateIfEmpty = (emptyBehavior == InvalidateIfEmpty);

    // invalidate range?
    if (endInternal() <= startInternal()) {
        setRange(KTextEditor::Range::invalid());
    }
}

void TextRange::setRange(KTextEditor::Range range)
{
    // avoid work if nothing changed!
    if (range == toRange()) {
        return;
    }

    // remember old line range
    const auto oldLineRange = toLineRange();

    // change start and end cursor
    m_start.setPosition(range.start());
    m_end.setPosition(range.end());

    // check if range now invalid, don't emit feedback here, will be handled below
    // otherwise you can't delete ranges in feedback!
    checkValidity(oldLineRange, false);

    // no attribute or feedback set, be done
    if (!m_attribute && !m_feedback) {
        return;
    }

    // get full range
    int startLineMin = oldLineRange.start();
    if (oldLineRange.start() == -1 || (m_start.lineInternal() != -1 && m_start.lineInternal() < oldLineRange.start())) {
        startLineMin = m_start.line();
    }

    int endLineMax = oldLineRange.end();
    if (oldLineRange.end() == -1 || m_end.lineInternal() > oldLineRange.end()) {
        endLineMax = m_end.lineInternal();
    }

    // notify buffer about attribute change, it will propagate the changes
    // notify right view
    m_buffer.notifyAboutRangeChange(m_view, {startLineMin, endLineMax}, m_attribute);

    // perhaps need to notify stuff!
    if (m_feedback) {
        // do this last: may delete this range
        if (!toRange().isValid()) {
            m_feedback->rangeInvalid(this);
        } else if (toRange().isEmpty()) {
            m_feedback->rangeEmpty(this);
        }
    }
}

void TextRange::setRange(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute)
{
    // FIXME: optimize
    setRange(range);
    setAttribute(attribute);
}

void TextRange::setRange(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute, qreal zDepth)
{
    // FIXME: optimize
    setRange(range);
    setAttribute(attribute);
    setZDepth(zDepth);
}

void TextRange::checkValidity(KTextEditor::LineRange oldLineRange, bool notifyAboutChange)
{
    // in any case: reset the flag, to avoid multiple runs
    m_isCheckValidityRequired = false;

    // check if any cursor is invalid or the range is zero size and it should be invalidated then
    if (!m_start.isValid() || !m_end.isValid() || (m_invalidateIfEmpty && m_end <= m_start)) {
        m_start.setPosition(-1, -1);
        m_end.setPosition(-1, -1);
    }

    // for ranges which are allowed to become empty, normalize them, if the end has moved to the front of the start
    if (!m_invalidateIfEmpty && m_end < m_start) {
        m_end.setPosition(m_start);
    }

    // fix lookup
    fixLookup(oldLineRange, toLineRange());

    // perhaps need to notify stuff!
    if (notifyAboutChange && m_feedback) {
        m_buffer.notifyAboutRangeChange(m_view, toLineRange(), false /* attribute not interesting here */);

        // do this last: may delete this range
        if (!toRange().isValid()) {
            m_feedback->rangeInvalid(this);
        } else if (toRange().isEmpty()) {
            m_feedback->rangeEmpty(this);
        }
    }
}

void TextRange::fixLookup(KTextEditor::LineRange oldLineRange, KTextEditor::LineRange lineRange)
{
    // nothing changed?
    if (oldLineRange == lineRange) {
        return;
    }

    // now, not both can be invalid
    Q_ASSERT(oldLineRange.start() >= 0 || lineRange.start() >= 0);
    Q_ASSERT(oldLineRange.end() >= 0 || lineRange.end() >= 0);

    // get full range
    int startLineMin = oldLineRange.start();
    if (oldLineRange.start() == -1 || (lineRange.start() != -1 && lineRange.start() < oldLineRange.start())) {
        startLineMin = lineRange.start();
    }

    int endLineMax = oldLineRange.end();
    if (oldLineRange.end() == -1 || lineRange.end() > oldLineRange.end()) {
        endLineMax = lineRange.end();
    }

    // get start block
    int blockIdx = m_buffer.blockForLine(startLineMin);
    Q_ASSERT(blockIdx >= 0);

    // remove this range from m_ranges
    auto it = m_buffer.m_blocks.begin() + blockIdx;
    auto end = m_buffer.m_blocks.end();
    for (; it != end; ++it) {
        // either insert or remove range
        TextBlock *block = *it;
        if ((lineRange.end() < block->startLine()) || (lineRange.start() >= (block->startLine() + block->lines()))) {
            block->removeRange(this);
        } else {
            block->updateRange(this);
        }

        // ok, reached end block
        if (endLineMax < (block->startLine() + block->lines())) {
            return;
        }
    }

    // we should not be here, really, then endLine is wrong
    Q_ASSERT(false);
}

void TextRange::setView(KTextEditor::View *view)
{
    // nothing changes, nop
    if (view == m_view) {
        return;
    }

    // remember the new attribute
    m_view = view;

    // notify buffer about attribute change, it will propagate the changes
    // notify all views (can be optimized later)
    if (m_attribute || m_feedback) {
        m_buffer.notifyAboutRangeChange(nullptr, toLineRange(), m_attribute);
    }
}

void TextRange::setAttribute(KTextEditor::Attribute::Ptr attribute)
{
    // nothing changes, nop, only pointer compare
    if (attribute == m_attribute) {
        return;
    }

    // remember the new attribute
    m_attribute = attribute;

    // notify buffer about attribute change, it will propagate the changes
    // notify right view
    m_buffer.notifyAboutRangeChange(m_view, toLineRange(), true /* even for nullptr attribute, we had before one => repaint */);
}

void TextRange::setFeedback(KTextEditor::MovingRangeFeedback *feedback)
{
    // nothing changes, nop
    if (feedback == m_feedback) {
        return;
    }

    // remember the new feedback object
    m_feedback = feedback;

    // notify buffer about feedback change, it will propagate the changes
    // notify right view
    m_buffer.notifyAboutRangeChange(m_view, toLineRange(), m_attribute);
}

void TextRange::setAttributeOnlyForViews(bool onlyForViews)
{
    // just set the value, no need to trigger updates, printing is not interruptable
    m_attributeOnlyForViews = onlyForViews;
}

void TextRange::setZDepth(qreal zDepth)
{
    // nothing changes, nop
    if (zDepth == m_zDepth) {
        return;
    }

    // remember the new attribute
    m_zDepth = zDepth;

    // notify buffer about attribute change, it will propagate the changes
    if (m_attribute) {
        m_buffer.notifyAboutRangeChange(m_view, toLineRange(), m_attribute);
    }
}

KTextEditor::Document *Kate::TextRange::document() const
{
    return m_buffer.document();
}

}
