/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    Based on code of the SmartCursor/Range by:
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXTRANGE_H
#define KATE_TEXTRANGE_H

#include <ktexteditor/movingrange.h>

#include "katetextcursor.h"

namespace KTextEditor
{
class MovingRangeFeedback;
class View;
}

namespace Kate
{
class TextBuffer;

/**
 * Class representing a 'clever' text range.
 * It will automagically move if the text inside the buffer it belongs to is modified.
 * By intention no subclass of KTextEditor::Range, must be converted manually.
 * A TextRange is allowed to be empty. If you call setInvalidateIfEmpty(true),
 * a TextRange will become automatically invalid as soon as start() == end()
 * position holds.
 */
class TextRange final : public KTextEditor::MovingRange
{
    // this is a friend, block changes might invalidate ranges...
    friend class TextBlock;

public:
    /**
     * Construct a text range.
     * A TextRange is not allowed to be empty, as soon as start == end position, it will become
     * automatically invalid!
     * @param buffer parent text buffer
     * @param range The initial text range assumed by the new range.
     * @param insertBehavior Define whether the range should expand when text is inserted adjacent to the range.
     * @param emptyBehavior Define whether the range should invalidate itself on becoming empty.
     */
    TextRange(TextBuffer &buffer, KTextEditor::Range range, InsertBehaviors insertBehavior, EmptyBehavior emptyBehavior = AllowEmpty);

    /**
     * No copy constructor, don't allow this to be copied.
     */
    TextRange(const TextRange &) = delete;

    /**
     * Destruct the text block
     */
    ~TextRange() override;

    /**
     * No assignment operator, no copying around.
     */
    TextRange &operator=(const TextRange &) = delete;

    /**
     * Set insert behaviors.
     * @param insertBehaviors new insert behaviors
     */
    void setInsertBehaviors(InsertBehaviors insertBehaviors) override;

    /**
     * Get current insert behaviors.
     * @return current insert behaviors
     */
    InsertBehaviors insertBehaviors() const override;

    /**
     * Set if this range will invalidate itself if it becomes empty.
     * @param emptyBehavior behavior on becoming empty
     */
    void setEmptyBehavior(EmptyBehavior emptyBehavior) override;

    /**
     * Will this range invalidate itself if it becomes empty?
     * @return behavior on becoming empty
     */
    EmptyBehavior emptyBehavior() const override
    {
        return m_invalidateIfEmpty ? InvalidateIfEmpty : AllowEmpty;
    }

    /**
     * Gets the document to which this range is bound.
     * \return a pointer to the document
     */
    KTextEditor::Document *document() const override;

    /**
     * Set the range of this range.
     * A TextRange is not allowed to be empty, as soon as start == end position, it will become
     * automatically invalid!
     * @param range new range for this clever range
     */
    void setRange(KTextEditor::Range range) override;

    /**
     * \overload
     * Set the range of this range
     * A TextRange is not allowed to be empty, as soon as start == end position, it will become
     * automatically invalid!
     * @param start new start for this clever range
     * @param end new end for this clever range
     */
    void setRange(KTextEditor::Cursor start, KTextEditor::Cursor end)
    {
        KTextEditor::MovingRange::setRange(start, end);
    }

    /**
     * Set the range of this range and the connected attribute.
     * Avoids internal overhead of separate setting that.
     *
     * A TextRange is not allowed to be empty, as soon as start == end position, it will become
     * automatically invalid!
     *
     * @param range new range for this clever range
     * @param attribute Attribute to assign to this range. If null, simply removes the previous Attribute.
     *
     * @since 6.0
     */
    void setRange(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute) override;

    /**
     * Set the range of this range and the connected attribute and Z-depth.
     * Avoids internal overhead of separate setting that.
     *
     * A TextRange is not allowed to be empty, as soon as start == end position, it will become
     * automatically invalid!
     *
     * @param range new range for this clever range
     * @param attribute Attribute to assign to this range. If null, simply removes the previous Attribute.
     * @param zDepth new Z-depth of this range
     *
     * @since 6.0
     */
    void setRange(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute, qreal zDepth) override;

    /**
     * Retrieve start cursor of this range, read-only.
     * @return start cursor
     */
    const KTextEditor::MovingCursor &start() const override
    {
        return m_start;
    }

    /**
     * Non-virtual version of start(), which is faster.
     * @return start cursor
     */
    const TextCursor &startInternal() const
    {
        return m_start;
    }

    /**
     * Retrieve end cursor of this range, read-only.
     * @return end cursor
     */
    const KTextEditor::MovingCursor &end() const override
    {
        return m_end;
    }

    /**
     * Nonvirtual version of end(), which is faster.
     * @return end cursor
     */
    const TextCursor &endInternal() const
    {
        return m_end;
    }

    /**
     * Hides parent's impl of toLineRange() and uses non-virtual functions internally.
     */
    KTextEditor::LineRange toLineRange() const
    {
        return {startInternal().lineInternal(), endInternal().lineInternal()};
    }

    /**
     * Convert this clever range into a dumb one.
     * @return normal range
     */
    const KTextEditor::Range toRange() const
    {
        auto startCursor = KTextEditor::Cursor(startInternal().lineInternal(), startInternal().columnInternal());
        auto endCursor = KTextEditor::Cursor(endInternal().lineInternal(), endInternal().columnInternal());
        return KTextEditor::Range(startCursor, endCursor);
    }

    /**
     * Convert this clever range into a dumb one. Equal to toRange, allowing to use implicit conversion.
     * @return normal range
     */
    operator KTextEditor::Range() const
    {
        return toRange();
    }

    /**
     * Gets the active view for this range. Might be already invalid, internally only used for pointer comparisons.
     *
     * \return a pointer to the active view
     */
    KTextEditor::View *view() const override
    {
        return m_view;
    }

    /**
     * Sets the currently active view for this range.
     * This will trigger update of the relevant view parts, if the view changed.
     * Set view before the attribute, that will avoid not needed redraws.
     *
     * \param view View to assign to this range. If null, simply
     *                  removes the previous view.
     */
    void setView(KTextEditor::View *view) override;

    /**
     * Gets the active Attribute for this range.
     *
     * \return a pointer to the active attribute
     */
    const KTextEditor::Attribute::Ptr &attribute() const override
    {
        return m_attribute;
    }

    /**
     * \return whether a nonzero attribute is set. This is faster than checking attribute(),
     *             because the reference-counting is omitted.
     */
    bool hasAttribute() const
    {
        return m_attribute.constData();
    }

    /**
     * Sets the currently active attribute for this range.
     * This will trigger update of the relevant view parts.
     *
     * \param attribute Attribute to assign to this range. If null, simply
     *                  removes the previous Attribute.
     */
    void setAttribute(KTextEditor::Attribute::Ptr attribute) override;

    /**
     * Gets the active MovingRangeFeedback for this range.
     *
     * \return a pointer to the active MovingRangeFeedback
     */
    KTextEditor::MovingRangeFeedback *feedback() const override
    {
        return m_feedback;
    }

    /**
     * Sets the currently active MovingRangeFeedback for this range.
     * This will trigger evaluation if feedback must be send again (for example if mouse is already inside range).
     *
     * \param feedback MovingRangeFeedback to assign to this range. If null, simply
     *                  removes the previous MovingRangeFeedback.
     */
    void setFeedback(KTextEditor::MovingRangeFeedback *feedback) override;

    /**
     * Is this range's attribute only visible in views, not for example prints?
     * Default is false.
     * @return range visible only for views
     */
    bool attributeOnlyForViews() const override
    {
        return m_attributeOnlyForViews;
    }

    /**
     * Set if this range's attribute is only visible in views, not for example prints.
     * @param onlyForViews attribute only valid for views
     */
    void setAttributeOnlyForViews(bool onlyForViews) override;

    /**
     * Gets the current Z-depth of this range.
     * Ranges with smaller Z-depth than others will win during rendering.
     * Default is 0.0.
     *
     * \return current Z-depth of this range
     */
    qreal zDepth() const override
    {
        return m_zDepth;
    }

    /**
     * Set the current Z-depth of this range.
     * Ranges with smaller Z-depth than others will win during rendering.
     * This will trigger update of the relevant view parts, if the depth changed.
     * Set depth before the attribute, that will avoid not needed redraws.
     * Default is 0.0.
     *
     * \param zDepth new Z-depth of this range
     */
    void setZDepth(qreal zDepth) override;

private:
    /**
     * Check if range is valid, used by constructor and setRange.
     * If at least one cursor is invalid, both will set to invalid.
     * Same if range itself is invalid (start >= end).
     *
     * IMPORTANT: Notifications might need to deletion of this range!
     *
     * @param oldLineRange line range of this range before changing of cursors, needed to add/remove range from m_ranges in blocks, required!
     * @param notifyAboutChange should feedback be emitted or not?
     */
    void checkValidity(KTextEditor::LineRange oldLineRange, bool notifyAboutChange = true);

    /**
     * Add/Remove range from the lookup m_ranges hash of each block
     * @param oldLineRange old line range before changing of cursors, needed to add/remove range from m_ranges in blocks
     * @param lineRange line range to start looking for the range to remove
     */
    void fixLookup(KTextEditor::LineRange oldLineRange, KTextEditor::LineRange lineRange);

    /**
     * Mark this range for later validity checking.
     */
    void setValidityCheckRequired()
    {
        m_isCheckValidityRequired = true;
    }

    /**
     * Does this range need validity checking?
     * @return is checking required?
     */
    bool isValidityCheckRequired() const
    {
        return m_isCheckValidityRequired;
    }

private:
    /**
     * parent text buffer
     * is a reference, and no pointer, as this must always exist and can't change
     */
    TextBuffer &m_buffer;

    /**
     * Start cursor for this range, is a clever cursor
     */
    TextCursor m_start;

    /**
     * End cursor for this range, is a clever cursor
     */
    TextCursor m_end;

    /**
     * The view for which the attribute is valid, 0 means any view
     */
    KTextEditor::View *m_view;

    /**
     * This range's current attribute.
     */
    KTextEditor::Attribute::Ptr m_attribute;

    /**
     * pointer to the active MovingRangeFeedback
     */
    KTextEditor::MovingRangeFeedback *m_feedback;

    /**
     * Z-depth of this range for rendering
     */
    qreal m_zDepth;

    /**
     * Is this range's attribute only visible in views, not for example prints?
     */
    bool m_attributeOnlyForViews;

    /**
     * Will this range invalidate itself if it becomes empty?
     */
    bool m_invalidateIfEmpty;

    /**
     * Should this range be validated?
     * Used by KateTextBlock to avoid multiple updates without costly hashing.
     * Reset by checkValidity().
     */
    bool m_isCheckValidityRequired = false;
};

}

#endif
