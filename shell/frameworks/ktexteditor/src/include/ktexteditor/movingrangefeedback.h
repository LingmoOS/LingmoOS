/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    Based on code of the SmartCursor/Range by:
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_MOVINGRANGEFEEDBACK_H
#define KTEXTEDITOR_MOVINGRANGEFEEDBACK_H

#include <ktexteditor_export.h>

namespace KTextEditor
{
class View;
class MovingRange;

/**
 * \class MovingRangeFeedback movingrangefeedback.h <KTextEditor/MovingRangeFeedback>
 *
 * \short A class which provides notifications of state changes to a MovingRange.
 *
 * \ingroup kte_group_moving_classes
 *
 * This class provides notifications of changes to the position or contents of a MovingRange.
 *
 * Before destruction, you must unregister the feedback class from any range using it.
 *
 * \author Christoph Cullmann \<cullmann@kde.org\>
 *
 * \since 4.5
 */
class KTEXTEDITOR_EXPORT MovingRangeFeedback
{
public:
    /**
     * Default constructor
     */
    MovingRangeFeedback();

    /**
     * Virtual destructor
     */
    virtual ~MovingRangeFeedback();

    /**
     * The range is now empty (ie. the start and end cursors are the same).
     * If the range has invalidateIfEmpty set, this will never be emitted, but instead rangeInvalid is triggered.
     * You may delete the range inside this method, but don't alter the range here (for example by using setRange).
     *
     * \param range pointer to the range which generated the notification.
     */
    virtual void rangeEmpty(MovingRange *range);

    /**
     * The range is now invalid (ie. the start and end cursors are invalid).
     * You may delete the range inside this method, but don't alter the range here (for example by using setRange).
     *
     * \param range pointer to the range which generated the notification.
     */
    virtual void rangeInvalid(MovingRange *range);

    /**
     * The mouse cursor on \a view entered \p range.
     *
     * \param range pointer to the range which generated the notification.
     * \param view view over which the mouse moved to generate the notification
     */
    virtual void mouseEnteredRange(MovingRange *range, View *view);

    /**
     * The mouse cursor on \a view exited \p range.
     *
     * \param range pointer to the range which generated the notification.
     * \param view view over which the mouse moved to generate the notification
     */
    virtual void mouseExitedRange(MovingRange *range, View *view);

    /**
     * The caret on \a view entered \p range.
     *
     * \param range pointer to the range which generated the notification.
     * \param view view over which the mouse moved to generate the notification
     */
    virtual void caretEnteredRange(MovingRange *range, View *view);

    /**
     * The caret on \a view exited \p range.
     *
     * \param range pointer to the range which generated the notification.
     * \param view view over which the mouse moved to generate the notification
     */
    virtual void caretExitedRange(MovingRange *range, View *view);

private:
    /**
     * private d-pointer
     */
    class MovingRangeFeedbackPrivate *const d = nullptr;
};

}

#endif
